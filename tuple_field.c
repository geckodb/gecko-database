#include <tuple_field.h>
#include <grid.h>

void gs_tuple_field_open(tuple_field_t *field, tuple_t *tuple)
{
    require_non_null(field);
    require_non_null(tuple);

    attr_id_t attr_id = 0;

    grid_id_t grid_id = gs_grid_table_grid_by_field(tuple->table, attr_id, tuple->tuple_id);
    const grid_t *grid = gs_grid_by_id(tuple->table, grid_id);

    tuplet_t *tuplet = gs_tuplet_open(grid->frag);
    tuplet_field_t *tuplet_field = gs_tuplet_field_open(tuplet);

    *field = (tuple_field_t) {
        .tuple = tuple,
        .table_attr_id = attr_id,
        .grid = grid,
        .tuplet = tuplet,
        .field = tuplet_field,
        .frag_attr_id = gs_grid_table_attr_id_to_frag_attr_id(grid, attr_id)
    };
}

void gs_tuple_field_next(tuple_field_t *field)
{

}

void gs_tuple_field_write(tuple_field_t *field, const void *data)
{

}
