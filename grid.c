#include <grid.h>
#include <indexes/vindexes/hash_vindex.h>
#include <indexes/hindexes/lsearch_hindex.h>
#include <containers/dicts/hash_table.h>

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
        grid_table_t *result = require_good_malloc(sizeof(grid_table_t));
        result->schema = gs_schema_cpy(schema);
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
    gs_vindex_free(table->schema_cover);
    gs_hindex_free(table->tuple_cover);

   // panic(NOTIMPLEMENTED, to_string(gs_grid_table_free))
}

void gs_grid_free(grid_t * grid)
{
    gs_fragment_free(grid->frag);
    dict_free(grid->schema_map_indicies);
    vector_free(grid->tuple_ids);
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

grid_set_cursor_t *gs_grid_table_grid_find(const grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids,
                                  const tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    grid_set_cursor_t *v_result = gs_vindex_query(table->schema_cover, attr_ids, attr_ids + nattr_ids);
    grid_set_cursor_t *h_result = gs_hindex_query(table->tuple_cover, tuple_ids, tuple_ids + ntuple_ids);

    bool v_less_h = (grid_set_cursor_numelem(v_result) < grid_set_cursor_numelem(h_result));
    grid_set_cursor_t *smaller = v_less_h ? v_result : h_result;
    grid_set_cursor_t *larger  = v_less_h ? h_result : v_result;

    grid_set_cursor_t *result = grid_set_cursor_create(grid_set_cursor_numelem(larger));

    /* Hash-join intersection */
    dict_t* hash_table = hash_table_create_jenkins(sizeof(grid_t *), sizeof(bool),
                                                   2 * grid_set_cursor_numelem(smaller), 1.7f, 0.75f);

    /* Build */
    for (const grid_t *grid = grid_set_cursor_next(smaller); grid != NULL; grid = grid_set_cursor_next(NULL)) {
        bool b;
        dict_put(hash_table, &grid, &b);
    }

    /* Probe */
    for (const grid_t *grid = grid_set_cursor_next(larger); grid != NULL; grid = grid_set_cursor_next(NULL)) {
        if (dict_contains_key(hash_table, &grid)) {
            grid_set_cursor_pushback(result, &grid);
        }
    }

    gs_vindex_query_close(v_result);
    gs_hindex_query_close(h_result);

    return result;
}

// This function returns NULL, if the table attribute is not covered by this grid
const attr_id_t *gs_grid_table_attr_id_to_frag_attr_id(const grid_t *grid, attr_id_t table_attr_id)
{
    require_non_null(grid);
    const attr_id_t *frag_attr_id = dict_get(grid->schema_map_indicies, &table_attr_id);
    return frag_attr_id;
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
    tuple_id_t *tuple_ids = require_good_malloc(ntuplets * sizeof(tuple_id_t));
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
    grid_t *result = require_good_malloc(sizeof(grid_t));

    schema_t *grid_schema = gs_schema_subset(table->schema, attr, nattr);
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