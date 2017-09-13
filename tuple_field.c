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

#include <tuple_field.h>
#include <grid.h>
#include <schema.h>

void gs_tuple_field_open(tuple_field_t *field, tuple_t *tuple)
{
    gs_tuple_field_seek(field, tuple, 0);
}

void gs_tuple_field_seek(tuple_field_t *tuple_field, tuple_t *tuple, attr_id_t table_attr_id)
{
    REQUIRE_NONNULL(tuple_field);
    REQUIRE_NONNULL(tuple);

    grid_cursor_t *cursor = gs_grid_table_grid_find(tuple->table, &table_attr_id, 1, &tuple->tuple_id, 1);
    REQUIRE_WARGS((grid_cursor_numelem(cursor) == 1),
                  "Internal error: tuple_field [tuple #%d @ '%s'] is covered by %zu grids. Must be covered by "
                  "exactly one grid instead.", tuple->tuple_id, gs_grid_table_attr_by_id(tuple->table,
                                                                                         table_attr_id)->name,
                  grid_cursor_numelem(cursor));
    grid_t *grid = grid_cursor_next(cursor);

    gs_tuplet_open(&tuple_field->tuplet, grid->frag, gs_grid_global_to_local(grid, tuple->tuple_id, AT_RANDOM));

    attr_id_t grid_attr_id = *gs_grid_table_attr_id_to_frag_attr_id(grid, table_attr_id);

    tuplet_field_t tuplet_field;
    gs_tuplet_field_seek(&tuplet_field, &tuple_field->tuplet, grid_attr_id);

    tuple_field->tuple = tuple;
    tuple_field->table_attr_id = table_attr_id;
    tuple_field->grid_attr_id = grid_attr_id;
    tuple_field->grid = grid;
    tuple_field->tuplet_field = tuplet_field;

    grid_cursor_close(cursor);
}

void gs_tuple_field_next(tuple_field_t *field)
{
    const attr_id_t *attr_id = gs_grid_table_attr_id_to_frag_attr_id(field->grid, ++field->table_attr_id);
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

void gs_tuple_field_write(tuple_field_t *field, const void *data)
{
    gs_tuplet_field_write(&field->tuplet_field, data, false);
    gs_tuple_field_next(field);
}

const void *gs_tuple_field_read(tuple_field_t *field)
{
    return gs_tuplet_field_read(&field->tuplet_field);
}