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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdinc.h>
#include <grid_cursor.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    GI_VINDEX_HASH,

    GI_HINDEX_BESEARCH
} grid_index_tag;

typedef struct grid_cursor_t {
    void *extra;
} grid_cursor_t;

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

grid_cursor_t *grid_cursor_create(size_t cursor);

void grid_cursor_close(grid_cursor_t *cursor);

void grid_cursor_pushback(grid_cursor_t *cursor, const void *data);

void grid_cursor_append(grid_cursor_t *dst, grid_cursor_t *src);

void grid_cursor_dedup(grid_cursor_t *cursor);

struct grid_t *grid_cursor_next(grid_cursor_t *cursor);

size_t grid_cursor_numelem(const grid_cursor_t *cursor);

bool gs_grid_cursor_is_empty(const grid_cursor_t *cursor);
