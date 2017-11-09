// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_grid.h>
#include <indexes/vindexes/gs_hash_vindex.h>
#include <indexes/hindexes/gs_lsearch_hindex.h>
#include <gs_schema.h>
#include <gs_tuplet_field.h>
#include <gs_tuple_field.h>
#include <apr_strings.h>
#include <gs_tuple_field.h>

void create_indexes(gs_table_t *table, size_t approx_num_horizontal_partitions);

 void create_gs_grid_ptr_store(gs_table_t *table);

 void create_gs_tuple_id_store(gs_table_t *table);

 gs_grid_t *create_grid(gs_table_t *table, const gs_attr_id_t *attr, size_t nattr,
                                  const gs_tuple_id_interval_t *tuple_ids, size_t ntuple_ids, enum gs_frag_impl_type_e type);

 size_t get_required_capacity(const gs_tuple_id_interval_t *tuple_ids, size_t ntuple_ids);

 void indexes_insert(gs_table_t *table, gs_grid_t *grid, const gs_attr_id_t *attr, size_t nattr,
                                  const gs_tuple_id_interval_t *tuples, size_t ntuples);

 void register_grid(gs_table_t *table, gs_grid_t *grid);

gs_table_t *gs_table_new(const gs_schema_t *schema, size_t approx_num_horizontal_partitions)
{
    if (schema != NULL) {
        gs_table_t *result = GS_REQUIRE_MALLOC(sizeof(gs_table_t));
        result->schema = gs_schema_cpy(schema);
        result->num_tuples = 0;
        create_indexes(result, approx_num_horizontal_partitions);
        create_gs_grid_ptr_store(result);
        create_gs_tuple_id_store(result);
        return result;
    } else return NULL;
}

 bool free_grids(void *capture, void *begin, void *end)
{
    for (gs_grid_t **it = (gs_grid_t **) begin; it < (gs_grid_t **) end; it++) {
        gs_grid_delete(*it);
        free(*it);
    }
    return true;
}

void gs_table_delete(gs_table_t *table)
{
    gs_schema_delete(table->schema);
    gs_vec_foreach(table->grid_ptrs, NULL, free_grids);
    gs_vec_free(table->grid_ptrs);
    gs_vindex_delete(table->schema_cover);
    gs_hindex_delete(table->tuple_cover);
    gs_freelist_dispose(&table->tuple_id_freelist);
    free(table->schema_cover);
    free(table->tuple_cover);
}

void gs_grid_delete(gs_grid_t *grid)
{
    gs_frag_delete(grid->frag);
    apr_pool_destroy(grid->pool);
    gs_vec_free(grid->tuple_ids);
}

const char *gs_table_name(const gs_table_t *table)
{
    GS_REQUIRE_NONNULL(table);
    return (table->schema->frag_name);
}

gs_grid_id_t gs_table_add(gs_table_t *table, const gs_attr_id_t *attr_ids_covered, size_t nattr_ids_covered,
                    const gs_tuple_id_interval_t *tuple_ids_covered, size_t ntuple_ids_covered, enum gs_frag_impl_type_e type)
{
    GS_REQUIRE_NONNULL(table);
    GS_REQUIRE_NONNULL(table->schema);
    GS_REQUIRE_NONNULL(attr_ids_covered);
    GS_REQUIRE_NONNULL(tuple_ids_covered);

    gs_grid_t *grid = create_grid(table, attr_ids_covered, nattr_ids_covered, tuple_ids_covered, ntuple_ids_covered, type);
    indexes_insert(table, grid, attr_ids_covered, nattr_ids_covered, tuple_ids_covered, ntuple_ids_covered);
    register_grid(table, grid);

    // Determine the maximum number of tuples in this table
    while (ntuple_ids_covered--) {
        REQUIRE_LESSTHAN(tuple_ids_covered->begin, tuple_ids_covered->end);
        table->num_tuples = max(table->num_tuples, tuple_ids_covered->end);
        tuple_ids_covered++;
    }

    // Return the grid identifier of the newly added grid. Since it's accessed as an index in the 'grid_ptrs' vector,
    // just return the index of the new grid in this vector
    return (table->grid_ptrs->num_elements - 1);
}

const gs_freelist_t *gs_table_freelist(const struct gs_table_t *table)
{
    GS_REQUIRE_NONNULL(table);
    return &(table->tuple_id_freelist);
}

gs_grid_cursor_t *gs_table_find(const gs_table_t *table, const gs_attr_id_t *attr_ids, size_t nattr_ids,
                          const gs_tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    gs_grid_cursor_t *v_result = gs_vindex_query(table->schema_cover, attr_ids, attr_ids + nattr_ids);
    gs_grid_cursor_t *h_result = gs_hindex_query(table->tuple_cover, tuple_ids, tuple_ids + ntuple_ids);

    bool v_less_h = (gs_grid_cursor_numelem(v_result) < gs_grid_cursor_numelem(h_result));
    gs_grid_cursor_t *smaller = v_less_h ? v_result : h_result;
    gs_grid_cursor_t *larger  = v_less_h ? h_result : v_result;

    gs_grid_cursor_t *result = gs_grid_cursor_new(gs_grid_cursor_numelem(larger));

    /* Hash-join intersection */
    panic_if(gs_grid_cursor_is_empty(smaller), "No grid found. Does the table field cover for %p contain gaps?", table);

    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);
    bool *dummy = GS_REQUIRE_MALLOC(sizeof(dummy));
    *dummy = true;

    apr_hash_t* hash_table = apr_hash_make(pool);
    //sizeof(gs_grid_t *), sizeof(bool), 10 * gs_grid_cursor_numelem(smaller), 1.7f, 0.95f);

    /* Build */
    for (const gs_grid_t *grid = gs_grid_cursor_next(smaller); grid != NULL; grid = gs_grid_cursor_next(NULL)) {
        gs_grid_t *gs_grid_cp = apr_pmemdup(pool, grid, sizeof(gs_grid_t));
        apr_hash_set(hash_table, gs_grid_cp, sizeof(gs_grid_t), dummy);
    }

    /* Probe */
    for (const gs_grid_t *grid = gs_grid_cursor_next(larger); grid != NULL; grid = gs_grid_cursor_next(NULL)) {
        if (apr_hash_get(hash_table, &grid, sizeof(gs_grid_t))) {
            gs_grid_cursor_pushback(result, &grid);
        }
    }

    apr_pool_destroy(pool);
    gs_vindex_close(v_result);
    gs_hindex_close(h_result);

    return result;
}

gs_table_t *gs_table_melt(enum gs_frag_impl_type_e type, const gs_table_t *src_table, const gs_tuple_id_t *tuple_ids,
                    size_t ntuple_ids, const gs_attr_id_t *attr_ids, size_t nattr_ids)
{
    gs_tuple_t src_tuple, dst_tuple;
    gs_tuple_field_t src_field, dst_field;
    gs_tuple_cursor_t dst_cursor;
    gs_tuple_id_t src_gs_tuple_id = 0;

    gs_schema_t *dst_schema = gs_schema_subset(src_table->schema, attr_ids, nattr_ids);
    gs_table_t *dst_table = gs_table_new(dst_schema, 1);
    gs_tuple_id_interval_t cover = { .begin = 0, .end = ntuple_ids };
    gs_table_add(dst_table, attr_ids, nattr_ids, &cover, 1, type);
    gs_grid_insert(&dst_cursor, dst_table, ntuple_ids);

    while (gs_tuple_cursor_next(&dst_tuple, &dst_cursor)) {
        gs_tuple_open(&src_tuple, src_table, src_gs_tuple_id++);
        for (size_t x = 0; x < nattr_ids; x++) {
            gs_attr_id_t attr_id = attr_ids[x];
            gs_tuple_field_seek(&src_field, &src_tuple, attr_id);
            gs_tuple_field_seek(&dst_field, &dst_tuple, attr_id);
            const void *field_data = gs_tuple_field_read(&src_field);
            gs_tuple_field_write(&dst_field, field_data);
        }
    }
    gs_tuple_cursor_dispose(&dst_cursor);
    gs_schema_delete(dst_schema);
    return dst_table;
}

// This function returns NULL, if the table attribute is not covered by this grid
const gs_attr_id_t *gs_table_attr_id_to_frag_attr_id(const gs_grid_t *grid, gs_attr_id_t gs_table_attr_id)
{
    GS_REQUIRE_NONNULL(grid);
    const gs_attr_id_t *gs_frag_attr_id = apr_hash_get(grid->schema_map_indicies, &gs_table_attr_id, sizeof(gs_attr_id_t));
    return gs_frag_attr_id;
}

const gs_attr_t *gs_table_attr_by_id(const gs_table_t *table, gs_attr_id_t id)
{
    return gs_schema_attr_by_id(table->schema, id);
}

const char *gs_table_attr_name_by_id(const gs_table_t *table, gs_attr_id_t id)
{
    return gs_table_attr_by_id(table, id)->name;
}

size_t gs_table_num_of_attributes(const gs_table_t *table)
{
    GS_REQUIRE_NONNULL(table)
    return table->schema->attr->num_elements;
}

size_t gs_table_num_of_tuples(const gs_table_t *table)
{
    GS_REQUIRE_NONNULL(table)
    return table->num_tuples;
}

size_t gs_table_num_of_grids(const gs_table_t *table)
{
    GS_REQUIRE_NONNULL(table)
    return table->grid_ptrs->num_elements;
}

const gs_grid_t *gs_grid_by_id(const gs_table_t *table, gs_grid_id_t id)
{
    GS_REQUIRE_NONNULL(table);
    REQUIRE_LESSTHAN(id, table->grid_ptrs->num_elements);
    return *(const gs_grid_t **) gs_vec_at(table->grid_ptrs, id);
}

size_t gs_grid_num_of_attributes(const gs_grid_t *grid)
{
    GS_REQUIRE_NONNULL(grid);
    return grid->frag->schema->attr->num_elements;
}

gs_vec_t *gs_table_grids_by_attr(const gs_table_t *table, const gs_attr_id_t *attr_ids, size_t nattr_ids)
{
    panic(NOTIMPLEMENTED, to_string(gs_table_grids_by_attr))
    return NULL;
}

gs_vec_t *gs_table_grids_by_tuples(const gs_table_t *table, const gs_tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    panic(NOTIMPLEMENTED, to_string(gs_table_grids_by_tuples))
    return NULL;
}

bool gs_table_is_valide(gs_table_t *table)
{
    panic(NOTIMPLEMENTED, to_string(gs_table_is_valide))
    return false;
}

void gs_grid_insert(gs_tuple_cursor_t *resultset, gs_table_t *table, size_t ntuplets)
{
    GS_REQUIRE_NONNULL(table);
    GS_REQUIRE_NONNULL(resultset);
    REQUIRE((ntuplets > 0), BADINT);
    gs_tuple_id_t *tuple_ids = GS_REQUIRE_MALLOC(ntuplets * sizeof(gs_tuple_id_t));
    gs_freelist_bind(tuple_ids, &table->tuple_id_freelist, ntuplets);
    gs_tuple_cursor_create(resultset, table, tuple_ids, ntuplets);
}

void gs_grid_print(FILE *file, const gs_table_t *table, gs_grid_id_t gs_grid_id, size_t row_offset, size_t limit)
{
    GS_REQUIRE_NONNULL(file)
    GS_REQUIRE_NONNULL(table)
    const gs_grid_t *grid = gs_grid_by_id(table, gs_grid_id);
    gs_frag_print(stdout, grid->frag, row_offset, limit);
}

void gs_table_gs_grid_list_print(FILE *file, const gs_table_t *table, size_t row_offset, size_t limit)
{
    gs_tuplet_t tuplet;

    gs_schema_t *print_schema = gs_schema_new("ad hoc info");
    attr_create_gridid("grid id", print_schema);
    attr_create_tformat("tuple format", print_schema);
    attr_create_fragtype("implementation", print_schema);
    attr_create_uint64("tuplet count", print_schema);
    attr_create_size("tuplet capacity", print_schema);
    attr_create_size("tuplet size", print_schema);
    attr_create_size("total size", print_schema);

    size_t num_tuples = gs_table_num_of_grids(table);
    gs_frag_t *frag = gs_frag_new(print_schema, num_tuples, FIT_HOST_NSM_VM);
    gs_frag_insert(&tuplet, frag, num_tuples);

    do {
        gs_tuplet_field_t field;
        gs_tuplet_field_open(&field, &tuplet);
        for (size_t i = 0; i < num_tuples; i++) {
            const gs_grid_t *grid = gs_grid_by_id(table, i);
            gs_tuplet_field_write(&field, &i, true);
            gs_tuplet_field_write(&field, &grid->frag->format, true);
            gs_tuplet_field_write(&field, &grid->frag->impl_type, true);
            gs_tuplet_field_write(&field, &grid->frag->ntuplets, true);
            gs_tuplet_field_write(&field, &grid->frag->ncapacity, true);
            gs_tuplet_field_write(&field, &grid->frag->tuplet_size, true);
            size_t total_size = (grid->frag->tuplet_size * grid->frag->ncapacity);
            gs_tuplet_field_write(&field, &total_size, true);
        }
    } while (gs_tuplet_next(&tuplet));

    gs_frag_print(file, frag, 0, INT_MAX);

    gs_frag_delete(frag);
    gs_schema_delete(print_schema);
}

void gs_table_print(FILE *file, const gs_table_t *table, size_t row_offset, size_t limit)
{
    GS_REQUIRE_NONNULL(table);

    gs_tuple_id_t *tuple_ids = GS_REQUIRE_MALLOC(table->num_tuples * sizeof(gs_tuple_id_t));
    gs_attr_id_t *attr_ids = GS_REQUIRE_MALLOC(gs_table_num_of_attributes(table) * sizeof(gs_attr_id_t));

    for (size_t i = 0; i < table->num_tuples; tuple_ids[i] = i, i++);
    for (size_t i = 0; i < gs_table_num_of_attributes(table); attr_ids[i] = i, i++);

    gs_table_t *molten_table = gs_table_melt(FIT_HOST_NSM_VM, table, tuple_ids, table->num_tuples, attr_ids,
                                       gs_table_num_of_attributes(table));

    gs_frag_print(file, gs_grid_by_id(molten_table, 0)->frag, row_offset, limit);

    free(attr_ids);
    free(tuple_ids);
    gs_table_delete(molten_table);
    free(molten_table);
}

void gs_table_structure_print(FILE *file, const gs_table_t *table, size_t row_offset, size_t limit)
{
    gs_tuple_t read_tuple;
    gs_tuple_field_t read_field;
    gs_schema_t *write_schema;
    gs_tuplet_t write_tuplet;
    gs_frag_t *write_frag;
    gs_tuplet_field_t write_field;
    size_t num_tuples;
    size_t num_attr;

    num_tuples   = gs_table_num_of_tuples(table);
    num_attr     = gs_table_num_of_attributes(table);
    write_schema = gs_schema_new("ad hoc info");

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        attr_create_gridid(gs_table_attr_name_by_id(table, attr_idx), write_schema);
    }

    write_frag   = gs_frag_new(write_schema, num_tuples, FIT_HOST_NSM_VM);
    gs_frag_insert(&write_tuplet, write_frag, num_tuples);

    /* For each field in the input grid table, find the grid object that contains this field. The search is done via
     * nested linear search, currently. That's very naive, but does the job for now. */
    do {
        //gs_tuplet_field_open(&write_tuplet);
        gs_tuple_open(&read_tuple, table, write_tuplet.tuplet_id);
        gs_tuple_field_open(&read_field, &read_tuple);
        num_attr = gs_table_num_of_attributes(table);

        while (num_attr--) {
            const char *t_attr_name = gs_table_attr_name_by_id(table, read_field.table_attr_id);
            for (gs_grid_id_t gs_grid_id = 0; gs_grid_id < gs_table_num_of_grids(table); gs_grid_id++) {
                const gs_grid_t *grid = gs_grid_by_id(table, gs_grid_id);
                for (gs_attr_id_t gs_grid_attr_id = 0; gs_grid_attr_id < gs_grid_num_of_attributes(grid); gs_grid_attr_id++) {
                    const char *g_attr_name = gs_schema_attr_by_id(gs_frag_schema(grid->frag), gs_grid_attr_id)->name;
                    if (strcmp(t_attr_name, g_attr_name) == 0) {
                        for (size_t i = 0; i < grid->tuple_ids->num_elements; i++) {
                            const gs_tuple_id_interval_t *span = gs_vec_at(grid->tuple_ids, i);
                            if (INTERVAL_CONTAINS(span, write_tuplet.tuplet_id)) {
                                gs_tuplet_field_seek(&write_field, &write_tuplet, read_field.table_attr_id);
                                gs_tuplet_field_write(&write_field, &gs_grid_id, false);
                            }
                        }
                    }
                }
            }
            gs_tuple_field_next(&read_field);
        }
    } while (gs_tuplet_next(&write_tuplet));

    gs_frag_print(file, write_frag, row_offset, limit);

    gs_frag_delete(write_frag);
    gs_schema_delete(write_schema);
}

 void create_indexes(gs_table_t *table, size_t approx_num_horizontal_partitions)
{
    size_t num_gs_schema_slots = max(1 , 2 * table->schema->attr->num_elements);
    table->schema_cover = gs_hash_vindex_new(num_gs_schema_slots);
    table->tuple_cover  = gs_lesearch_hindex_new(approx_num_horizontal_partitions, table->schema);
}

 void create_gs_grid_ptr_store(gs_table_t *table)
{
    table->grid_ptrs = gs_vec_new(sizeof(gs_grid_t *), 10);
}

 void create_gs_tuple_id_store(gs_table_t *table)
{
    gs_freelist_create(&table->tuple_id_freelist, sizeof(gs_tuple_id_t), 100, gs_tuple_id_init, gs_tuple_id_inc);
}

 gs_grid_t *create_grid(gs_table_t *table, const gs_attr_id_t *attr, size_t nattr,
                                  const gs_tuple_id_interval_t *tuple_ids, size_t ntuple_ids, enum gs_frag_impl_type_e type)
{
    gs_grid_t *result = GS_REQUIRE_MALLOC(sizeof(gs_grid_t));

    gs_schema_t *gs_grid_schema = gs_schema_subset(table->schema, attr, nattr);
    assert (gs_grid_schema);
    size_t gs_tuplet_capacity = get_required_capacity(tuple_ids, ntuple_ids);

    apr_pool_create(&result->pool, NULL);

    *result = (gs_grid_t) {
        .context = table,
        .frag = gs_frag_new(gs_grid_schema, gs_tuplet_capacity, type),
        .schema_map_indicies = apr_hash_make(result->pool),
        .tuple_ids = gs_vec_new(sizeof(gs_tuple_id_interval_t), ntuple_ids),
        .last_interval_cache = NULL
            // TODO: add mutex init here
    };

    for (size_t i = 0; i < ntuple_ids; i++) {
        gs_frag_insert(NULL, result->frag, INTERVAL_SPAN((tuple_ids + i)));
    }

    gs_vec_pushback(result->tuple_ids, ntuple_ids, tuple_ids);

    for (size_t i = 0; i < nattr; i++) {
        gs_attr_id_t *key = apr_pmemdup(result->pool, (attr + i), sizeof(gs_attr_id_t));
        size_t *val = apr_pmemdup(result->pool, &i, sizeof(size_t));
        apr_hash_set(result->schema_map_indicies, key, sizeof(gs_attr_id_t), val);
    }

    gs_schema_delete(gs_grid_schema);

    return result;
}

 size_t get_required_capacity(const gs_tuple_id_interval_t *tuple_ids, size_t ntuple_ids)
{
    size_t capacity = 0;
    while (ntuple_ids--) {
        const gs_tuple_id_interval_t *interval = tuple_ids++;
        capacity += INTERVAL_SPAN(interval);
    }
    return capacity;
}

 void indexes_insert(gs_table_t *table, gs_grid_t *grid, const gs_attr_id_t *attr, size_t nattr,
                                  const gs_tuple_id_interval_t *tuples, size_t ntuples)
{
    while (nattr--) {
        gs_vindex_add(table->schema_cover, attr++, grid);
    }
    while (ntuples--) {
        gs_hindex_add(table->tuple_cover, tuples++, grid);
    }
}

 void register_grid(gs_table_t *table, gs_grid_t *grid)
{
    gs_vec_pushback(table->grid_ptrs, 1, &grid);
    grid->grid_id = gs_vec_length(table->grid_ptrs) - 1;
}