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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gecko-commons/gecko-commons.h>
#include <gs_grid_cursor.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    GI_VINDEX_HASH,

    GI_HINDEX_BESEARCH
} gs_grid_index_tag_e;

typedef struct gs_grid_cursor_t {
    void *extra;
} gs_grid_cursor_t;

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

gs_grid_cursor_t *gs_grid_cursor_new(size_t cursor);
void gs_grid_cursor_delete(gs_grid_cursor_t *cursor);
void gs_grid_cursor_pushback(gs_grid_cursor_t *cursor, const void *data);
struct gs_grid_t *gs_grid_cursor_next(gs_grid_cursor_t *cursor);
size_t gs_grid_cursor_numelem(const gs_grid_cursor_t *cursor);
bool gs_grid_cursor_is_empty(const gs_grid_cursor_t *cursor);
