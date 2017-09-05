#include <tuple_field.h>
#include <grid.h>
#include <schema.h>

void gs_tuple_field_open(tuple_field_t *field, tuple_t *tuple)
{
    return gs_tuple_field_seek(field, tuple, 0);
}

void gs_tuple_field_seek(tuple_field_t *field, tuple_t *tuple, attr_id_t attr_id)
{
    require_non_null(field);
    require_non_null(tuple);

    grid_set_cursor_t *cursor = gs_grid_table_grid_find(tuple->table, &attr_id, 1, &tuple->tuple_id, 1);
    require((grid_set_cursor_numelem(cursor) == 1), "Internal error: field is covered not exactly by one grid");
    grid_t *grid = grid_set_cursor_next(cursor);

    tuplet_t *tuplet = gs_tuplet_open(grid->frag, gs_grid_global_to_local(grid, tuple->tuple_id, AT_RANDOM));
    tuplet_field_t *tuplet_field = gs_tuplet_field_open(tuplet);

    *field = (tuple_field_t) {
            .tuple = tuple,
            .table_attr_id = attr_id,
            .grid = grid,
            .tuplet = tuplet,
            .field = tuplet_field,
            .frag_attr_id = *gs_grid_table_attr_id_to_frag_attr_id(grid, attr_id)
    };

    grid_set_cursor_close(cursor);
}

void gs_tuple_field_next(tuple_field_t *field)
{
    const attr_id_t *attr_id = gs_grid_table_attr_id_to_frag_attr_id(field->grid, ++field->table_attr_id);
    if (attr_id) {
        // next tuple field is inside the current tuplet
        gs_tuplet_field_seek(field->tuplet, *attr_id);
    } else {
        // next tuple fiels is in another tuplet (i.e., requires to search the other grid)
        if (field->table_attr_id < field->grid->context->schema->attr->num_elements) {
            // there are is at least one attribute in the table schema left that is covered by some grid, so proceed.
            gs_tuple_field_seek(field, field->tuple, field->table_attr_id);
        } else {
            // the tuple field cursor goes beyond the table schema attribute list, so close the cursor.
            gs_tuple_field_close(field);
        }
    }
}

void gs_tuple_field_write(tuple_field_t *field, const void *data)
{
    gs_tuplet_field_write(field->field, data);
    gs_tuple_field_next(field);
}

void gs_tuple_field_close(tuple_field_t *field)
{
    gs_tuplet_field_close(field->field);
}