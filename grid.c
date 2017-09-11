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
    gs_fragment_free(grid->frag);
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
    vector_add(table->grid_ptrs, 1, &grid);

    // Determine the maximum number of tuples in this table
    while (ntuple_ids_covered--) {
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
    dict_t* hash_table = hash_table_create_jenkins(sizeof(grid_t *), sizeof(bool),
                                                   2 * grid_cursor_numelem(smaller), 1.7f, 0.75f);

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

         /*   // DEBUG:
            if (src_field.tuplet_field->attr_id == 0) {
                printf("read tuplet (%u) tuplet_field attr0 (%llu): '%llu'\n", src_field.tuplet_field->tuplet->tuplet_id, src_field.tuplet_field->attr_id, *(u64*) src_field.tuplet_field->attr_value_ptr);
            } else if (src_field.tuplet_field->attr_id == 1) {
                printf("read tuplet (%u) tuplet_field attr1 (%llu): '%d'\n", src_field.tuplet_field->tuplet->tuplet_id, src_field.tuplet_field->attr_id, *(u32*) src_field.tuplet_field->attr_value_ptr);
            } else if (src_field.tuplet_field->attr_id == 2) {
                printf("read tuplet (%u) tuplet_field attr2 (%llu): '%d'\n", src_field.tuplet_field->tuplet->tuplet_id, src_field.tuplet_field->attr_id, *(u16*) src_field.tuplet_field->attr_value_ptr);
            } else if (src_field.tuplet_field->attr_id == 3) {
                printf("read tuplet (%u) tuplet_field attr3 (%llu): '%d'\n", src_field.tuplet_field->tuplet->tuplet_id, src_field.tuplet_field->attr_id, *(u16*) src_field.tuplet_field->attr_value_ptr);
            }*/

            gs_tuple_field_write(&dst_field, field_data);
        }
    }
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

size_t gs_grid_table_num_of_attributes(const grid_table_t *table)
{
    REQUIRE_NONNULL(table)
    return table->schema->attr->num_elements;
}

const grid_t *gs_grid_by_id(const grid_table_t *table, grid_id_t id)
{
    REQUIRE_NONNULL(table);
    REQUIRE_LESSTHAN(id, table->grid_ptrs->num_elements);
    return *(const grid_t **) vector_at(table->grid_ptrs, id);
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
    gs_schema_print(stdout, table->schema);
    gs_schema_print(stdout, grid_schema);
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
        gs_fragment_insert(NULL, result->frag, gs_interval_get_span((tuple_ids + i)));
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