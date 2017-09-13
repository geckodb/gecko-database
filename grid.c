// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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

#include <grid.h>
#include <indexes/vindexes/hash_vindex.h>
#include <indexes/hindexes/lsearch_hindex.h>
#include <containers/dicts/hash_table.h>
#include <schema.h>
#include <tuplet_field.h>
#include <tuple_field.h>

static inline void create_indexes(grid_table_t *table, size_t approx_num_horizontal_partitions);

static inline void create_grid_ptr_store(grid_table_t *table);

static inline void create_tuple_id_store(grid_table_t *table);

static inline grid_t *create_grid(grid_table_t *table, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuple_ids, size_t ntuple_ids, enum frag_impl_type_t type);

static inline size_t get_required_capacity(const tuple_id_interval_t *tuple_ids, size_t ntuple_ids);

static inline void indexes_insert(grid_table_t *table, grid_t *grid, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuples, size_t ntuples);

static inline void register_grid(grid_table_t *table, grid_t *grid);

grid_table_t *gs_grid_table_create(const schema_t *schema, size_t approx_num_horizontal_partitions)
{
    if (schema != NULL) {
        grid_table_t *result = REQUIRE_MALLOC(sizeof(grid_table_t));
        result->schema = gs_schema_cpy(schema);
        result->num_tuples = 0;
        create_indexes(result, approx_num_horizontal_partitions);
        create_grid_ptr_store(result);
        create_tuple_id_store(result);
        return result;
    } else return NULL;
}

static inline bool free_grids(void *capture, void *begin, void *end)
{
    for (grid_t **it = (grid_t **) begin; it < (grid_t **) end; it++) {
        gs_grid_free(*it);
        free(*it);
    }
    return true;
}

void gs_grid_table_free(grid_table_t *table)
{
    gs_schema_free(table->schema);
    vector_foreach(table->grid_ptrs, NULL, free_grids);
    vector_free(table->grid_ptrs);
    gs_vindex_free(table->schema_cover);
    gs_hindex_free(table->tuple_cover);
    gs_freelist_free(&table->tuple_id_freelist);
    free(table->schema_cover);
    free(table->tuple_cover);
}

void gs_grid_free(grid_t * grid)
{
    gs_frag_free(grid->frag);
    dict_free(grid->schema_map_indicies);
    vector_free(grid->tuple_ids);
}

const char *gs_grid_table_name(const grid_table_t *table)
{
    REQUIRE_NONNULL(table);
    return (table->schema->frag_name);
}

grid_id_t gs_grid_table_add_grid(grid_table_t *table, const attr_id_t *attr_ids_covered, size_t nattr_ids_covered,
                                 const tuple_id_interval_t *tuple_ids_covered, size_t ntuple_ids_covered, enum frag_impl_type_t type)
{
    REQUIRE_NONNULL(table);
    REQUIRE_NONNULL(table->schema);
    REQUIRE_NONNULL(attr_ids_covered);
    REQUIRE_NONNULL(tuple_ids_covered);

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

const freelist_t *gs_grid_table_freelist(const struct grid_table_t *table)
{
    REQUIRE_NONNULL(table);
    return &(table->tuple_id_freelist);
}

grid_cursor_t *gs_grid_table_grid_find(const grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids,
                                  const tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    grid_cursor_t *v_result = gs_vindex_query(table->schema_cover, attr_ids, attr_ids + nattr_ids);
    grid_cursor_t *h_result = gs_hindex_query(table->tuple_cover, tuple_ids, tuple_ids + ntuple_ids);

    bool v_less_h = (grid_cursor_numelem(v_result) < grid_cursor_numelem(h_result));
    grid_cursor_t *smaller = v_less_h ? v_result : h_result;
    grid_cursor_t *larger  = v_less_h ? h_result : v_result;

    grid_cursor_t *result = grid_cursor_create(grid_cursor_numelem(larger));

    /* Hash-join intersection */
    panic_if(gs_grid_cursor_is_empty(smaller), "No grid found. Does the table field cover for %p contain gaps?", table);
    dict_t* hash_table = hash_table_create_jenkins(sizeof(grid_t *), sizeof(bool),
                                                   10 * grid_cursor_numelem(smaller), 1.7f, 0.95f);

    /* Build */
    for (const grid_t *grid = grid_cursor_next(smaller); grid != NULL; grid = grid_cursor_next(NULL)) {
        bool b;
        dict_put(hash_table, &grid, &b);
    }

    /* Probe */
    for (const grid_t *grid = grid_cursor_next(larger); grid != NULL; grid = grid_cursor_next(NULL)) {
        if (dict_contains_key(hash_table, &grid)) {
            grid_cursor_pushback(result, &grid);
        }
    }

    dict_free(hash_table);
    gs_vindex_query_close(v_result);
    gs_hindex_query_close(h_result);

    return result;
}

grid_table_t *gs_grid_table_melt(enum frag_impl_type_t type, const grid_table_t *src_table, const tuple_id_t *tuple_ids,
                           size_t ntuple_ids, const attr_id_t *attr_ids, size_t nattr_ids)
{
    tuple_t src_tuple, dst_tuple;
    tuple_field_t src_field, dst_field;
    tuple_cursor_t dst_cursor;
    tuple_id_t src_tuple_id = 0;

    schema_t *dst_schema = gs_schema_subset(src_table->schema, attr_ids, nattr_ids);
    grid_table_t *dst_table = gs_grid_table_create(dst_schema, 1);
    tuple_id_interval_t cover = { .begin = 0, .end = ntuple_ids };
    gs_grid_table_add_grid(dst_table, attr_ids, nattr_ids, &cover, 1, type);
    gs_grid_table_insert(&dst_cursor, dst_table, ntuple_ids);

    while (gs_tuple_cursor_next(&dst_tuple, &dst_cursor)) {
        gs_tuple_open(&src_tuple, src_table, src_tuple_id++);
        for (size_t x = 0; x < nattr_ids; x++) {
            attr_id_t attr_id = attr_ids[x];
            gs_tuple_field_seek(&src_field, &src_tuple, attr_id);
            gs_tuple_field_seek(&dst_field, &dst_tuple, attr_id);
            const void *field_data = gs_tuple_field_read(&src_field);
            gs_tuple_field_write(&dst_field, field_data);
        }
    }
    gs_tuple_cursor_free(&dst_cursor);
    gs_schema_free(dst_schema);
    return dst_table;
}

// This function returns NULL, if the table attribute is not covered by this grid
const attr_id_t *gs_grid_table_attr_id_to_frag_attr_id(const grid_t *grid, attr_id_t table_attr_id)
{
    REQUIRE_NONNULL(grid);
    const attr_id_t *frag_attr_id = dict_get(grid->schema_map_indicies, &table_attr_id);
    return frag_attr_id;
}

const attr_t *gs_grid_table_attr_by_id(const grid_table_t *table, attr_id_t id)
{
    return gs_schema_attr_by_id(table->schema, id);
}

const char *gs_grid_table_attr_name_by_id(const grid_table_t *table, attr_id_t id)
{
    return gs_grid_table_attr_by_id(table, id)->name;
}

size_t gs_grid_table_num_of_attributes(const grid_table_t *table)
{
    REQUIRE_NONNULL(table)
    return table->schema->attr->num_elements;
}

size_t gs_grid_table_num_of_tuples(const grid_table_t *table)
{
    REQUIRE_NONNULL(table)
    return table->num_tuples;
}

size_t gs_grid_table_num_of_grids(const grid_table_t *table)
{
    REQUIRE_NONNULL(table)
    return table->grid_ptrs->num_elements;
}

const grid_t *gs_grid_by_id(const grid_table_t *table, grid_id_t id)
{
    REQUIRE_NONNULL(table);
    REQUIRE_LESSTHAN(id, table->grid_ptrs->num_elements);
    return *(const grid_t **) vector_at(table->grid_ptrs, id);
}

size_t gs_grid_num_of_attributes(const grid_t *grid)
{
    REQUIRE_NONNULL(grid);
    return grid->frag->schema->attr->num_elements;
}

vector_t *gs_grid_table_grids_by_attr(const grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_grids_by_attr))
    return NULL;
}

vector_t *gs_grid_table_grids_by_tuples(const grid_table_t *table, const tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_grids_by_tuples))
    return NULL;
}

bool gs_grid_table_is_valide(grid_table_t *table)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_is_valide))
    return false;
}

void gs_grid_table_insert(tuple_cursor_t *resultset, grid_table_t *table, size_t ntuplets)
{
    REQUIRE_NONNULL(table);
    REQUIRE_NONNULL(resultset);
    REQUIRE((ntuplets > 0), BADINT);
    tuple_id_t *tuple_ids = REQUIRE_MALLOC(ntuplets * sizeof(tuple_id_t));
    gs_freelist_bind(tuple_ids, &table->tuple_id_freelist, ntuplets);
    gs_tuple_cursor_create(resultset, table, tuple_ids, ntuplets);
}

void gs_grid_print(FILE *file, const grid_table_t *table, grid_id_t grid_id, size_t row_offset, size_t limit)
{
    REQUIRE_NONNULL(file)
    REQUIRE_NONNULL(table)
    const grid_t *grid = gs_grid_by_id(table, grid_id);
    gs_frag_print(stdout, grid->frag, row_offset, limit);
}

void gs_grid_table_grid_list_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit)
{
    tuplet_t tuplet;

    schema_t *print_schema = gs_schema_create("ad hoc info");
    gs_attr_create_gridid("grid id", print_schema);
    gs_attr_create_tformat("tuple format", print_schema);
    gs_attr_create_fragtype("implementation", print_schema);
    gs_attr_create_uint64("tuplet count", print_schema);
    gs_attr_create_size("tuplet capacity", print_schema);
    gs_attr_create_size("tuplet size", print_schema);
    gs_attr_create_size("total size", print_schema);

    size_t num_tuples = gs_grid_table_num_of_grids(table);
    frag_t *frag = gs_fragment_alloc(print_schema, num_tuples, FIT_HOST_NSM_VM);
    gs_frag_insert(&tuplet, frag, num_tuples);

    do {
        tuplet_field_t field;
        gs_tuplet_field_open(&field, &tuplet);
        for (size_t i = 0; i < num_tuples; i++) {
            const grid_t *grid = gs_grid_by_id(table, i);
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

    gs_frag_free(frag);
    gs_schema_free(print_schema);
}

void gs_grid_table_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit)
{
    REQUIRE_NONNULL(table);

    tuple_id_t *tuple_ids = REQUIRE_MALLOC(table->num_tuples * sizeof(tuple_id_t));
    attr_id_t *attr_ids = REQUIRE_MALLOC(gs_grid_table_num_of_attributes(table) * sizeof(attr_id_t));

    for (size_t i = 0; i < table->num_tuples; tuple_ids[i] = i, i++);
    for (size_t i = 0; i < gs_grid_table_num_of_attributes(table); attr_ids[i] = i, i++);

    grid_table_t *molten_table = gs_grid_table_melt(FIT_HOST_NSM_VM, table, tuple_ids, table->num_tuples, attr_ids,
                                      gs_grid_table_num_of_attributes(table));

    gs_frag_print(file, gs_grid_by_id(molten_table, 0)->frag, row_offset, limit);

    free(attr_ids);
    free(tuple_ids);
    gs_grid_table_free(molten_table);
    free(molten_table);
}

void gs_grid_table_structure_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit)
{
    tuple_t read_tuple;
    tuple_field_t read_field;
    schema_t *write_schema;
    tuplet_t write_tuplet;
    frag_t *write_frag;
    tuplet_field_t write_field;
    size_t num_tuples;
    size_t num_attr;

    num_tuples   = gs_grid_table_num_of_tuples(table);
    num_attr     = gs_grid_table_num_of_attributes(table);
    write_schema = gs_schema_create("ad hoc info");

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        gs_attr_create_gridid(gs_grid_table_attr_name_by_id(table, attr_idx), write_schema);
    }

    write_frag   = gs_fragment_alloc(write_schema, num_tuples, FIT_HOST_NSM_VM);
    gs_frag_insert(&write_tuplet, write_frag, num_tuples);

    /* For each field in the input grid table, find the grid object that contains this field. The search is done via
     * nested linear search, currently. That's very naive, but does the job for now. */
    do {
        //gs_tuplet_field_open(&write_tuplet);
        gs_tuple_open(&read_tuple, table, write_tuplet.tuplet_id);
        gs_tuple_field_open(&read_field, &read_tuple);
        num_attr = gs_grid_table_num_of_attributes(table);

        while (num_attr--) {
            const char *t_attr_name = gs_grid_table_attr_name_by_id(table, read_field.table_attr_id);
            for (grid_id_t grid_id = 0; grid_id < gs_grid_table_num_of_grids(table); grid_id++) {
                const grid_t *grid = gs_grid_by_id(table, grid_id);
                for (attr_id_t grid_attr_id = 0; grid_attr_id < gs_grid_num_of_attributes(grid); grid_attr_id++) {
                    const char *g_attr_name = gs_schema_attr_by_id(gs_frag_get_schema(grid->frag), grid_attr_id)->name;
                    if (strcmp(t_attr_name, g_attr_name) == 0) {
                        for (size_t i = 0; i < grid->tuple_ids->num_elements; i++) {
                            const tuple_id_interval_t *span = vector_at(grid->tuple_ids, i);
                            if (GS_INTERVAL_CONTAINS(span, write_tuplet.tuplet_id)) {
                                gs_tuplet_field_seek(&write_field, &write_tuplet, read_field.table_attr_id);
                                gs_tuplet_field_write(&write_field, &grid_id, false);
                            }
                        }
                    }
                }
            }
            gs_tuple_field_next(&read_field);
        }
    } while (gs_tuplet_next(&write_tuplet));

    gs_frag_print(file, write_frag, row_offset, limit);

    gs_frag_free(write_frag);
    gs_schema_free(write_schema);
}

static inline void create_indexes(grid_table_t *table, size_t approx_num_horizontal_partitions)
{
    size_t num_schema_slots = 2 * table->schema->attr->num_elements;
    table->schema_cover = hash_vindex_create(sizeof(attr_id_t), num_schema_slots);
    table->tuple_cover  = lesearch_hindex_create(approx_num_horizontal_partitions, table->schema);
}

static inline void create_grid_ptr_store(grid_table_t *table)
{
    table->grid_ptrs = vector_create(sizeof(grid_t *), 10);
}

static inline void create_tuple_id_store(grid_table_t *table)
{
    gs_freelist_create(&table->tuple_id_freelist, sizeof(tuple_id_t), 100, gs_tuple_id_init, gs_tuple_id_inc);
}

static inline grid_t *create_grid(grid_table_t *table, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuple_ids, size_t ntuple_ids, enum frag_impl_type_t type)
{
    grid_t *result = REQUIRE_MALLOC(sizeof(grid_t));

    schema_t *grid_schema = gs_schema_subset(table->schema, attr, nattr);
    assert (grid_schema);
    size_t tuplet_capacity = get_required_capacity(tuple_ids, ntuple_ids);

    *result = (grid_t) {
        .context = table,
        .frag = gs_fragment_alloc(grid_schema, tuplet_capacity, type),
        .schema_map_indicies = hash_table_create(
                &(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen}, sizeof(attr_id_t), sizeof(attr_id_t),
                nattr + 1, 1.7f, 1.0f
        ),
        .tuple_ids = vector_create(sizeof(tuple_id_interval_t), ntuple_ids),
        .last_interval_cache = NULL
            // TODO: add mutex init here
    };

    for (size_t i = 0; i < ntuple_ids; i++) {
        gs_frag_insert(NULL, result->frag, gs_interval_get_span((tuple_ids + i)));
    }

    vector_add(result->tuple_ids, ntuple_ids, tuple_ids);

    for (size_t i = 0; i < nattr; i++) {
        dict_put(result->schema_map_indicies, attr + i, &i);
    }

    gs_schema_free(grid_schema);

    return result;
}

static inline size_t get_required_capacity(const tuple_id_interval_t *tuple_ids, size_t ntuple_ids)
{
    size_t capacity = 0;
    while (ntuple_ids--) {
        const tuple_id_interval_t *interval = tuple_ids++;
        capacity += gs_interval_get_span(interval);
    }
    return capacity;
}

static inline void indexes_insert(grid_table_t *table, grid_t *grid, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuples, size_t ntuples)
{
    while (nattr--) {
        gs_vindex_add(table->schema_cover, attr++, grid);
    }
    while (ntuples--) {
        gs_hindex_add(table->tuple_cover, tuples++, grid);
    }
}

static inline void register_grid(grid_table_t *table, grid_t *grid)
{
    vector_add(table->grid_ptrs, 1, &grid);
    grid->grid_id = vector_num_elements(table->grid_ptrs) - 1;
}