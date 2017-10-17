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

void create_indexes(table_t *table, size_t approx_num_horizontal_partitions);

 void create_grid_ptr_store(table_t *table);

 void create_tuple_id_store(table_t *table);

 grid_t *create_grid(table_t *table, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuple_ids, size_t ntuple_ids, enum frag_impl_type_t type);

 size_t get_required_capacity(const tuple_id_interval_t *tuple_ids, size_t ntuple_ids);

 void indexes_insert(table_t *table, grid_t *grid, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuples, size_t ntuples);

 void register_grid(table_t *table, grid_t *grid);

table_t *table_new(const schema_t *schema, size_t approx_num_horizontal_partitions)
{
    if (schema != NULL) {
        table_t *result = GS_REQUIRE_MALLOC(sizeof(table_t));
        result->schema = schema_cpy(schema);
        result->num_tuples = 0;
        create_indexes(result, approx_num_horizontal_partitions);
        create_grid_ptr_store(result);
        create_tuple_id_store(result);
        return result;
    } else return NULL;
}

 bool free_grids(void *capture, void *begin, void *end)
{
    for (grid_t **it = (grid_t **) begin; it < (grid_t **) end; it++) {
        grid_delete(*it);
        free(*it);
    }
    return true;
}

void table_delete(table_t *table)
{
    schema_delete(table->schema);
    vec_foreach(table->grid_ptrs, NULL, free_grids);
    vec_free(table->grid_ptrs);
    vindex_delete(table->schema_cover);
    hindex_delete(table->tuple_cover);
    freelist_dispose(&table->tuple_id_freelist);
    free(table->schema_cover);
    free(table->tuple_cover);
}

void grid_delete(grid_t *grid)
{
    frag_delete(grid->frag);
    apr_pool_destroy(grid->pool);
    vec_free(grid->tuple_ids);
}

const char *table_name(const table_t *table)
{
    GS_REQUIRE_NONNULL(table);
    return (table->schema->frag_name);
}

grid_id_t table_add(table_t *table, const attr_id_t *attr_ids_covered, size_t nattr_ids_covered,
                    const tuple_id_interval_t *tuple_ids_covered, size_t ntuple_ids_covered, enum frag_impl_type_t type)
{
    GS_REQUIRE_NONNULL(table);
    GS_REQUIRE_NONNULL(table->schema);
    GS_REQUIRE_NONNULL(attr_ids_covered);
    GS_REQUIRE_NONNULL(tuple_ids_covered);

    grid_t *grid = create_grid(table, attr_ids_covered, nattr_ids_covered, tuple_ids_covered, ntuple_ids_covered, type);
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

const freelist_t *table_freelist(const struct table_t *table)
{
    GS_REQUIRE_NONNULL(table);
    return &(table->tuple_id_freelist);
}

grid_cursor_t *table_find(const table_t *table, const attr_id_t *attr_ids, size_t nattr_ids,
                          const tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    grid_cursor_t *v_result = vindex_query(table->schema_cover, attr_ids, attr_ids + nattr_ids);
    grid_cursor_t *h_result = hindex_query(table->tuple_cover, tuple_ids, tuple_ids + ntuple_ids);

    bool v_less_h = (grid_cursor_numelem(v_result) < grid_cursor_numelem(h_result));
    grid_cursor_t *smaller = v_less_h ? v_result : h_result;
    grid_cursor_t *larger  = v_less_h ? h_result : v_result;

    grid_cursor_t *result = grid_cursor_new(grid_cursor_numelem(larger));

    /* Hash-join intersection */
    panic_if(grid_cursor_is_empty(smaller), "No grid found. Does the table field cover for %p contain gaps?", table);

    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);
    bool *dummy = GS_REQUIRE_MALLOC(sizeof(dummy));
    *dummy = true;

    apr_hash_t* hash_table = apr_hash_make(pool);
    //sizeof(grid_t *), sizeof(bool), 10 * grid_cursor_numelem(smaller), 1.7f, 0.95f);

    /* Build */
    for (const grid_t *grid = grid_cursor_next(smaller); grid != NULL; grid = grid_cursor_next(NULL)) {
        grid_t *grid_cp = apr_pmemdup(pool, grid, sizeof(grid_t));
        apr_hash_set(hash_table, grid_cp, sizeof(grid_t), dummy);
    }

    /* Probe */
    for (const grid_t *grid = grid_cursor_next(larger); grid != NULL; grid = grid_cursor_next(NULL)) {
        if (apr_hash_get(hash_table, &grid, sizeof(grid_t))) {
            grid_cursor_pushback(result, &grid);
        }
    }

    apr_pool_destroy(pool);
    vindex_close(v_result);
    hindex_close(h_result);

    return result;
}

table_t *table_melt(enum frag_impl_type_t type, const table_t *src_table, const tuple_id_t *tuple_ids,
                    size_t ntuple_ids, const attr_id_t *attr_ids, size_t nattr_ids)
{
    tuple_t src_tuple, dst_tuple;
    tuple_field_t src_field, dst_field;
    tuple_cursor_t dst_cursor;
    tuple_id_t src_tuple_id = 0;

    schema_t *dst_schema = schema_subset(src_table->schema, attr_ids, nattr_ids);
    table_t *dst_table = table_new(dst_schema, 1);
    tuple_id_interval_t cover = { .begin = 0, .end = ntuple_ids };
    table_add(dst_table, attr_ids, nattr_ids, &cover, 1, type);
    grid_insert(&dst_cursor, dst_table, ntuple_ids);

    while (tuple_cursor_next(&dst_tuple, &dst_cursor)) {
        tuple_open(&src_tuple, src_table, src_tuple_id++);
        for (size_t x = 0; x < nattr_ids; x++) {
            attr_id_t attr_id = attr_ids[x];
            tuple_field_seek(&src_field, &src_tuple, attr_id);
            tuple_field_seek(&dst_field, &dst_tuple, attr_id);
            const void *field_data = tuple_field_read(&src_field);
            tuple_field_write(&dst_field, field_data);
        }
    }
    tuple_cursor_dispose(&dst_cursor);
    schema_delete(dst_schema);
    return dst_table;
}

// This function returns NULL, if the table attribute is not covered by this grid
const attr_id_t *table_attr_id_to_frag_attr_id(const grid_t *grid, attr_id_t table_attr_id)
{
    GS_REQUIRE_NONNULL(grid);
    const attr_id_t *frag_attr_id = apr_hash_get(grid->schema_map_indicies, &table_attr_id, sizeof(attr_id_t));
    return frag_attr_id;
}

const attr_t *table_attr_by_id(const table_t *table, attr_id_t id)
{
    return schema_attr_by_id(table->schema, id);
}

const char *table_attr_name_by_id(const table_t *table, attr_id_t id)
{
    return table_attr_by_id(table, id)->name;
}

size_t table_num_of_attributes(const table_t *table)
{
    GS_REQUIRE_NONNULL(table)
    return table->schema->attr->num_elements;
}

size_t table_num_of_tuples(const table_t *table)
{
    GS_REQUIRE_NONNULL(table)
    return table->num_tuples;
}

size_t table_num_of_grids(const table_t *table)
{
    GS_REQUIRE_NONNULL(table)
    return table->grid_ptrs->num_elements;
}

const grid_t *grid_by_id(const table_t *table, grid_id_t id)
{
    GS_REQUIRE_NONNULL(table);
    REQUIRE_LESSTHAN(id, table->grid_ptrs->num_elements);
    return *(const grid_t **) vec_at(table->grid_ptrs, id);
}

size_t grid_num_of_attributes(const grid_t *grid)
{
    GS_REQUIRE_NONNULL(grid);
    return grid->frag->schema->attr->num_elements;
}

vec_t *table_grids_by_attr(const table_t *table, const attr_id_t *attr_ids, size_t nattr_ids)
{
    panic(NOTIMPLEMENTED, to_string(table_grids_by_attr))
    return NULL;
}

vec_t *table_grids_by_tuples(const table_t *table, const tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    panic(NOTIMPLEMENTED, to_string(table_grids_by_tuples))
    return NULL;
}

bool table_is_valide(table_t *table)
{
    panic(NOTIMPLEMENTED, to_string(table_is_valide))
    return false;
}

void grid_insert(tuple_cursor_t *resultset, table_t *table, size_t ntuplets)
{
    GS_REQUIRE_NONNULL(table);
    GS_REQUIRE_NONNULL(resultset);
    REQUIRE((ntuplets > 0), BADINT);
    tuple_id_t *tuple_ids = GS_REQUIRE_MALLOC(ntuplets * sizeof(tuple_id_t));
    freelist_bind(tuple_ids, &table->tuple_id_freelist, ntuplets);
    tuple_cursor_create(resultset, table, tuple_ids, ntuplets);
}

void grid_print(FILE *file, const table_t *table, grid_id_t grid_id, size_t row_offset, size_t limit)
{
    GS_REQUIRE_NONNULL(file)
    GS_REQUIRE_NONNULL(table)
    const grid_t *grid = grid_by_id(table, grid_id);
    frag_print(stdout, grid->frag, row_offset, limit);
}

void table_grid_list_print(FILE *file, const table_t *table, size_t row_offset, size_t limit)
{
    tuplet_t tuplet;

    schema_t *print_schema = schema_new("ad hoc info");
    attr_create_gridid("grid id", print_schema);
    attr_create_tformat("tuple format", print_schema);
    attr_create_fragtype("implementation", print_schema);
    attr_create_uint64("tuplet count", print_schema);
    attr_create_size("tuplet capacity", print_schema);
    attr_create_size("tuplet size", print_schema);
    attr_create_size("total size", print_schema);

    size_t num_tuples = table_num_of_grids(table);
    frag_t *frag = frag_new(print_schema, num_tuples, FIT_HOST_NSM_VM);
    frag_insert(&tuplet, frag, num_tuples);

    do {
        tuplet_field_t field;
        tuplet_field_open(&field, &tuplet);
        for (size_t i = 0; i < num_tuples; i++) {
            const grid_t *grid = grid_by_id(table, i);
            tuplet_field_write(&field, &i, true);
            tuplet_field_write(&field, &grid->frag->format, true);
            tuplet_field_write(&field, &grid->frag->impl_type, true);
            tuplet_field_write(&field, &grid->frag->ntuplets, true);
            tuplet_field_write(&field, &grid->frag->ncapacity, true);
            tuplet_field_write(&field, &grid->frag->tuplet_size, true);
            size_t total_size = (grid->frag->tuplet_size * grid->frag->ncapacity);
            tuplet_field_write(&field, &total_size, true);
        }
    } while (tuplet_next(&tuplet));

    frag_print(file, frag, 0, INT_MAX);

    frag_delete(frag);
    schema_delete(print_schema);
}

void table_print(FILE *file, const table_t *table, size_t row_offset, size_t limit)
{
    GS_REQUIRE_NONNULL(table);

    tuple_id_t *tuple_ids = GS_REQUIRE_MALLOC(table->num_tuples * sizeof(tuple_id_t));
    attr_id_t *attr_ids = GS_REQUIRE_MALLOC(table_num_of_attributes(table) * sizeof(attr_id_t));

    for (size_t i = 0; i < table->num_tuples; tuple_ids[i] = i, i++);
    for (size_t i = 0; i < table_num_of_attributes(table); attr_ids[i] = i, i++);

    table_t *molten_table = table_melt(FIT_HOST_NSM_VM, table, tuple_ids, table->num_tuples, attr_ids,
                                       table_num_of_attributes(table));

    frag_print(file, grid_by_id(molten_table, 0)->frag, row_offset, limit);

    free(attr_ids);
    free(tuple_ids);
    table_delete(molten_table);
    free(molten_table);
}

void table_structure_print(FILE *file, const table_t *table, size_t row_offset, size_t limit)
{
    tuple_t read_tuple;
    tuple_field_t read_field;
    schema_t *write_schema;
    tuplet_t write_tuplet;
    frag_t *write_frag;
    tuplet_field_t write_field;
    size_t num_tuples;
    size_t num_attr;

    num_tuples   = table_num_of_tuples(table);
    num_attr     = table_num_of_attributes(table);
    write_schema = schema_new("ad hoc info");

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        attr_create_gridid(table_attr_name_by_id(table, attr_idx), write_schema);
    }

    write_frag   = frag_new(write_schema, num_tuples, FIT_HOST_NSM_VM);
    frag_insert(&write_tuplet, write_frag, num_tuples);

    /* For each field in the input grid table, find the grid object that contains this field. The search is done via
     * nested linear search, currently. That's very naive, but does the job for now. */
    do {
        //tuplet_field_open(&write_tuplet);
        tuple_open(&read_tuple, table, write_tuplet.tuplet_id);
        tuple_field_open(&read_field, &read_tuple);
        num_attr = table_num_of_attributes(table);

        while (num_attr--) {
            const char *t_attr_name = table_attr_name_by_id(table, read_field.table_attr_id);
            for (grid_id_t grid_id = 0; grid_id < table_num_of_grids(table); grid_id++) {
                const grid_t *grid = grid_by_id(table, grid_id);
                for (attr_id_t grid_attr_id = 0; grid_attr_id < grid_num_of_attributes(grid); grid_attr_id++) {
                    const char *g_attr_name = schema_attr_by_id(frag_schema(grid->frag), grid_attr_id)->name;
                    if (strcmp(t_attr_name, g_attr_name) == 0) {
                        for (size_t i = 0; i < grid->tuple_ids->num_elements; i++) {
                            const tuple_id_interval_t *span = vec_at(grid->tuple_ids, i);
                            if (INTERVAL_CONTAINS(span, write_tuplet.tuplet_id)) {
                                tuplet_field_seek(&write_field, &write_tuplet, read_field.table_attr_id);
                                tuplet_field_write(&write_field, &grid_id, false);
                            }
                        }
                    }
                }
            }
            tuple_field_next(&read_field);
        }
    } while (tuplet_next(&write_tuplet));

    frag_print(file, write_frag, row_offset, limit);

    frag_delete(write_frag);
    schema_delete(write_schema);
}

 void create_indexes(table_t *table, size_t approx_num_horizontal_partitions)
{
    size_t num_schema_slots = 2 * table->schema->attr->num_elements;
    table->schema_cover = hash_vindex_new(sizeof(attr_id_t), num_schema_slots);
    table->tuple_cover  = lesearch_hindex_new(approx_num_horizontal_partitions, table->schema);
}

 void create_grid_ptr_store(table_t *table)
{
    table->grid_ptrs = vec_new(sizeof(grid_t *), 10);
}

 void create_tuple_id_store(table_t *table)
{
    freelist_create(&table->tuple_id_freelist, sizeof(tuple_id_t), 100, tuple_id_init, tuple_id_inc);
}

 grid_t *create_grid(table_t *table, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuple_ids, size_t ntuple_ids, enum frag_impl_type_t type)
{
    grid_t *result = GS_REQUIRE_MALLOC(sizeof(grid_t));

    schema_t *grid_schema = schema_subset(table->schema, attr, nattr);
    assert (grid_schema);
    size_t tuplet_capacity = get_required_capacity(tuple_ids, ntuple_ids);

    apr_pool_create(&result->pool, NULL);

    *result = (grid_t) {
        .context = table,
        .frag = frag_new(grid_schema, tuplet_capacity, type),
        .schema_map_indicies = apr_hash_make(result->pool),
        .tuple_ids = vec_new(sizeof(tuple_id_interval_t), ntuple_ids),
        .last_interval_cache = NULL
            // TODO: add mutex init here
    };

    for (size_t i = 0; i < ntuple_ids; i++) {
        frag_insert(NULL, result->frag, INTERVAL_SPAN((tuple_ids + i)));
    }

    vec_pushback(result->tuple_ids, ntuple_ids, tuple_ids);

    for (size_t i = 0; i < nattr; i++) {
        attr_id_t *key = apr_pmemdup(result->pool, (attr + i), sizeof(attr_id_t));
        size_t *val = apr_pmemdup(result->pool, &i, sizeof(size_t));
        apr_hash_set(result->schema_map_indicies, key, sizeof(attr_id_t), val);
    }

    schema_delete(grid_schema);

    return result;
}

 size_t get_required_capacity(const tuple_id_interval_t *tuple_ids, size_t ntuple_ids)
{
    size_t capacity = 0;
    while (ntuple_ids--) {
        const tuple_id_interval_t *interval = tuple_ids++;
        capacity += INTERVAL_SPAN(interval);
    }
    return capacity;
}

 void indexes_insert(table_t *table, grid_t *grid, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuples, size_t ntuples)
{
    while (nattr--) {
        vindex_add(table->schema_cover, attr++, grid);
    }
    while (ntuples--) {
        hindex_add(table->tuple_cover, tuples++, grid);
    }
}

 void register_grid(table_t *table, grid_t *grid)
{
    vec_pushback(table->grid_ptrs, 1, &grid);
    grid->grid_id = vec_length(table->grid_ptrs) - 1;
}