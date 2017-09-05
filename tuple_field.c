#include <tuple_field.h>
#include <grid.h>

void gs_tuple_field_open(tuple_field_t *field, tuple_t *tuple)
{
    require_non_null(field);
    require_non_null(tuple);

    attr_id_t attr_id = 0;

    grid_set_cursor_t *cursor = gs_grid_table_grid_find(tuple->table, &attr_id, 1, &tuple->tuple_id, 1);
    require((grid_set_cursor_numelem(cursor) == 1), "Internal error: field is covered not exactly by one grid");
    grid_t *grid = grid_set_cursor_next(cursor);

    // TODO: add to end, or uodate in-place

    tuplet_t *tuplet = gs_tuplet_open(grid->frag, gs_grid_global_to_local(grid, tuple->tuple_id, AT_RANDOM));
    tuplet_field_t *tuplet_field = gs_tuplet_field_open(tuplet);

    *field = (tuple_field_t) {
        .tuple = tuple,
        .table_attr_id = attr_id,
        .grid = grid,
        .tuplet = tuplet,
        .field = tuplet_field,
        .frag_attr_id = gs_grid_table_attr_id_to_frag_attr_id(grid, attr_id)
    };

    grid_set_cursor_close(cursor);
}

void gs_tuple_field_next(tuple_field_t *field)
{

}

void gs_tuple_field_write(tuple_field_t *field, const void *data)
{

}
