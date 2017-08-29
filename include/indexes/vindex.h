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
#include <indexes/grid_index.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct grid_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct vindex_t {
    grid_index_tag tag;

    void (*_add)(struct vindex_t *self, const void *key, const struct grid_t *grid);
    void (*_remove)(struct vindex_t *self, const void *key);
    bool (*_contains)(const struct vindex_t *self, const void *key);
    void (*_query)(grid_index_result_cursor_t *result, const struct vindex_t *self, const void *key_begin,
                                  const void *key_end);
    void (*_free)(struct vindex_t *self);

    void *extra;
} vindex_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void gs_vindex_free(vindex_t *index);

void gs_vindex_add(vindex_t *index, const void *key, const struct grid_t *grid);
void gs_vindex_remove(vindex_t *index, const void *key);
bool gs_vindex_contains(const vindex_t *index, const void *key);
grid_index_result_cursor_t *gs_vindex_query_open(const struct vindex_t *index, const void *key_range_begin,
                                      const void *key_range_end);
grid_index_result_cursor_t *gs_vindex_query_append(const struct vindex_t *index, grid_index_result_cursor_t *result,
                                        const void *key_range_begin, const void *key_range_end);
const struct grid_t *gs_vindex_query_read(grid_index_result_cursor_t *result_set);
void gs_vindex_query_close(grid_index_result_cursor_t *result_set);
