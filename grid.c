#include <grid.h>
#include <indexes/vindexes/hash_vindex.h>
#include <indexes/hindexes/bsearch_hindex.h>
#include <containers/dicts/hash_table.h>

static inline void create_indexes(grid_table_t *table, size_t approx_num_horizontal_partitions);

static inline void create_grid_ptr_store(grid_table_t *table);

static inline void create_tuple_id_store(grid_table_t *table);

static inline grid_t *create_grid(grid_table_t *table, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuple_ids, size_t ntuple_ids, enum frag_impl_type_t type);

static inline size_t get_required_capacity(const tuple_id_interval_t *tuple_ids, size_t ntuple_ids);

static inline void indexes_insert(grid_table_t *table, grid_t *grid, const attr_id_t *attr, size_t nattr,
                                  const tuple_id_interval_t *tuples, size_t ntuples);

static inline bool attr_key_equals(const void *key_lhs, const void *key_rhs);
static inline void attr_key_cleanup(void *key, void *value);

grid_table_t *gs_grid_table_create(const schema_t *schema, size_t approx_num_horizontal_partitions)
{
    if (schema != NULL) {
        grid_table_t *result = require_good_malloc(sizeof(grid_table_t));
        result->schema = gs_schema_cpy(schema);
        create_indexes(result, approx_num_horizontal_partitions);
        create_grid_ptr_store(result);
        create_tuple_id_store(result);
        return result;
    } else return NULL;
}

void gs_grid_table_free(grid_table_t *table)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_free))
}

const char *gs_grid_table_name(const grid_table_t *table)
{
    require_non_null(table);
    return (table->schema->frag_name);
}

grid_id_t gs_grid_table_add_grid(grid_table_t *table, const attr_id_t *attr_ids_covered, size_t nattr_ids_covered,
                                 const tuple_id_interval_t *tuple_ids_covered, size_t ntuple_ids_covered, enum frag_impl_type_t type)
{
    require_non_null(table);
    require_non_null(table->schema);
    require_non_null(attr_ids_covered);
    require_non_null(tuple_ids_covered);

    grid_t *grid = create_grid(table, attr_ids_covered, nattr_ids_covered, tuple_ids_covered, ntuple_ids_covered, type);
    indexes_insert(table, grid, attr_ids_covered, nattr_ids_covered, tuple_ids_covered, ntuple_ids_covered);
    vector_add(table->grid_ptrs, 1, &grid);

    return 0;
}

const freelist_t *gs_grid_table_freelist(const struct grid_table_t *table)
{
    require_non_null(table);
    return &(table->tuple_id_freelist);
}

grid_id_t gs_grid_table_grid_by_field(const grid_table_t *table, attr_id_t attr_id, tuple_id_t tuple_id)
{
    grid_index_result_cursor_t *v_result = gs_vindex_query_open(table->schema_cover, &attr_id, &attr_id + 1);
    grid_index_result_cursor_t *h_result = gs_hindex_query_open(table->tuple_cover, &tuple_id, &tuple_id + 1);

    for (const grid_t *grid = gs_vindex_query_read(v_result); grid != NULL;
         grid = gs_vindex_query_read(NULL)) {
    }

    for (const grid_t *grid = gs_hindex_query_read(h_result); grid != NULL;
         grid = gs_hindex_query_read(NULL)) {
    }

    gs_vindex_query_close(v_result);
    gs_hindex_query_close(h_result);

    panic(NOTIMPLEMENTED, to_string(gs_grid_table_grid_by_field))
    return 0;
}

attr_id_t gs_grid_table_attr_id_to_frag_attr_id(const grid_t *grid, attr_id_t table_attr_id)
{
    require_non_null(grid);
    require((table_attr_id >= grid->context->schema->attr->num_elements), ATTROUTOFBOUDNDS);
    const attr_id_t *frag_attr_id = dict_get(grid->schema_map_indicies, &table_attr_id);
    require_non_null(frag_attr_id);
    return *frag_attr_id;
}

const grid_t *gs_grid_by_id(const grid_table_t *table, grid_id_t id)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_by_id))
    return NULL;
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

void gs_grid_table_insert(resultset_t *resultset, grid_table_t *table, size_t ntuplets)
{
    require_non_null(table);
    require_non_null(resultset);
    require((ntuplets > 0), BADINT);
    tuple_id_t *tuple_ids = require_good_malloc(sizeof(tuple_id_t));
    gs_freelist_bind(tuple_ids, &table->tuple_id_freelist, ntuplets);
    gs_resultset_create(resultset, table, tuple_ids, ntuplets);
}

void gs_grid_table_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_print))
}

void gs_grid_table_grid_print(FILE *file, const grid_table_t *table, grid_id_t grid_id, size_t row_offset, size_t limit)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_grid_print))
}

static inline void create_indexes(grid_table_t *table, size_t approx_num_horizontal_partitions)
{
    size_t num_schema_slots = 2 * table->schema->attr->num_elements;
    table->schema_cover = hash_vindex_create(sizeof(attr_id_t), num_schema_slots,
                                             attr_key_equals, attr_key_cleanup);
    table->tuple_cover  = besearch_hindex_create(approx_num_horizontal_partitions, table->schema);
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
    grid_t *result = require_good_malloc(sizeof(grid_t));

    schema_t *grid_schema = gs_schema_subset(table->schema, attr, nattr);
    size_t tuplet_capacity = get_required_capacity(tuple_ids, ntuple_ids);

    *result = (grid_t) {
        .context = table,
        .frag = gs_fragment_alloc(grid_schema, tuplet_capacity, type),
        .schema_map_indicies = hash_table_create(
                &(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen}, sizeof(attr_id_t), sizeof(attr_id_t),
                nattr + 1, 1.7f, 1.0f
        )
        // TODO: add mutex init here
    };
    for (size_t i = 0; i < nattr; i++) {
        dict_put(result->schema_map_indicies, attr + i, &i);
    }

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

static inline bool attr_key_equals(const void *key_lhs, const void *key_rhs)
{
    attr_id_t lhs = *((attr_id_t *) key_lhs);
    attr_id_t rhs = *((attr_id_t *) key_rhs);
    return (lhs == rhs);
}

static inline void attr_key_cleanup(void *key, void *value)
{

}
