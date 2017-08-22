#include <grid.h>

//static inline void init_column_index(grid_table_t *table);

grid_table_t *gs_grid_table_create(const schema_t *schema)
{
    if (schema != NULL) {
//        grid_table_t *result = require_good_malloc(sizeof(grid_table_t));
        //init_column_index(result);

        // TODO:...
        return NULL;
    } else return NULL;
}

void gs_grid_table_free(grid_table_t *table)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_free))
}

grid_id_t gs_grid_table_add_grid(grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids,
                                 const interval_t *tuple_id_cover, size_t ntuple_id_cover, enum frag_impl_type_t type)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_add_grid))
    return 0;
}

grid_id_t gs_grid_table_grid_by_field(const grid_table_t *table, attr_id_t attr_id, tuple_id_t tuple_id)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_grid_by_field))
    return 0;
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

tuple_t *gs_grid_table_insert(grid_table_t *table, size_t ntuplets)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_insert))
    return NULL;
}

void gs_grid_table_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_print))
}

void gs_grid_table_grid_print(FILE *file, const grid_table_t *table, grid_id_t grid_id, size_t row_offset, size_t limit)
{
    panic(NOTIMPLEMENTED, to_string(gs_grid_table_grid_print))
}