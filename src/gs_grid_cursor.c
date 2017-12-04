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

#include <gecko-commons/containers/gs_vec.h>

#include <gs_grid_cursor.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_grid_cursor_t *gs_grid_cursor_new(size_t cursor)
{
    gs_grid_cursor_t *result = GS_REQUIRE_MALLOC(sizeof(gs_grid_cursor_t));
    *result = (gs_grid_cursor_t) {
            .extra = gs_vec_new(sizeof(struct gs_grid_t *), cursor)
    };
    return result;
}

void gs_grid_cursor_delete(gs_grid_cursor_t *cursor)
{
    gs_vec_free(cursor->extra);
    free (cursor);
}

void gs_grid_cursor_pushback(gs_grid_cursor_t *cursor, const void *data)
{
    gs_vec_pushback((cursor->extra), 1, data);
}

struct gs_grid_t *gs_grid_cursor_next(gs_grid_cursor_t *cursor)
{
    static gs_grid_cursor_t *dest;
    static size_t elem_idx;
    if (cursor != NULL) {
        dest = cursor;
        elem_idx = 0;
    }

    gs_vec_t *vec = (gs_vec_t * ) dest->extra;
    if (elem_idx < vec->num_elements) {
        return *(struct gs_grid_t **) gs_vec_at(vec, elem_idx++);
    } else return NULL;
}

size_t gs_grid_cursor_numelem(const gs_grid_cursor_t *cursor)
{
    GS_REQUIRE_NONNULL(cursor);
    return ((gs_vec_t * ) cursor->extra)->num_elements;
}

bool gs_grid_cursor_is_empty(const gs_grid_cursor_t *cursor)
{
    GS_REQUIRE_NONNULL(cursor)
    return (gs_vec_length(cursor->extra) == 0);
}