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

#include <gs_tuple_field.h>
#include <gs_grid.h>
#include <gs_schema.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_tuple_field_open(gs_tuple_field_t *field, gs_tuple_t *tuple)
{
    gs_tuple_field_seek(field, tuple, 0);
}

void gs_tuple_field_seek(gs_tuple_field_t *tuple_field, gs_tuple_t *tuple, gs_attr_id_t table_attr_id)
{
    GS_REQUIRE_NONNULL(tuple_field);
    GS_REQUIRE_NONNULL(tuple);

    gs_grid_cursor_t *cursor = gs_table_find(tuple->table, &table_attr_id, 1, &tuple->tuple_id, 1);
    REQUIRE_WARGS((gs_grid_cursor_numelem(cursor) == 1),
                  "Internal gs_gc_error: tuple_field [tuple #%d @ '%s'] is covered by %zu grids. Must be covered by "
                  "exactly one grid instead.", tuple->tuple_id, gs_table_attr_by_id(tuple->table,
                                                                                    table_attr_id)->name,
                  gs_grid_cursor_numelem(cursor));
    gs_grid_t *grid = gs_grid_cursor_next(cursor);

    gs_tuplet_open(&tuple_field->tuplet, grid->frag, gs_global_to_local(grid, tuple->tuple_id, AT_RANDOM));

    gs_attr_id_t grid_attr_id = *gs_table_attr_id_to_frag_attr_id(grid, table_attr_id);

    gs_tuplet_field_t tuplet_field;
    gs_tuplet_field_seek(&tuplet_field, &tuple_field->tuplet, grid_attr_id);

    tuple_field->tuple = tuple;
    tuple_field->table_attr_id = table_attr_id;
    tuple_field->grid_attr_id = grid_attr_id;
    tuple_field->grid = grid;
    tuple_field->tuplet_field = tuplet_field;

    gs_grid_cursor_delete(cursor);
}

void gs_tuple_field_next(gs_tuple_field_t *field)
{
    const gs_attr_id_t *attr_id = gs_table_attr_id_to_frag_attr_id(field->grid, ++field->table_attr_id);
    if (attr_id) {
        gs_tuplet_field_seek(&field->tuplet_field, &field->tuplet, *attr_id);
    } else {
        // next tuple field is in another tuplet (i.e., requires to search the other grid)
        if (field->table_attr_id < field->grid->context->schema->attr->num_elements) {
            // there are is at least one attribute in the table schema left that is covered by some grid, so proceed.
            gs_tuple_field_seek(field, field->tuple, field->table_attr_id);
        }
    }
}

void gs_tuple_field_write(gs_tuple_field_t *field, const void *data)
{
    gs_tuplet_field_write(&field->tuplet_field, data, false);
    gs_tuple_field_next(field);
}

const void *gs_tuple_field_read(gs_tuple_field_t *field)
{
    return gs_tuplet_field_read(&field->tuplet_field);
}