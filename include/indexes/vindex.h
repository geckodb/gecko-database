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
#include <containers/hashset.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct grid_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct vindex_t {
    grid_index_tag tag;
    hashset_t keys;

    void (*_add)(struct vindex_t *self, const attr_id_t *key, const struct grid_t *grid);
    void (*_remove)(struct vindex_t *self, const attr_id_t *key);
    bool (*_contains)(const struct vindex_t *self, const attr_id_t *key);
    void (*_query)(grid_cursor_t *result, const struct vindex_t *self, const attr_id_t *key_begin,
                                  const attr_id_t *key_end);
    void (*_free)(struct vindex_t *self);

    void *extra;
} vindex_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void vindex_delete(vindex_t *index);
void vindex_add(vindex_t *index, const attr_id_t *key, const struct grid_t *grid);
void vindex_remove(vindex_t *index, const attr_id_t *key);
bool vindex_contains(const vindex_t *index, const attr_id_t *key);
grid_cursor_t *vindex_query(const struct vindex_t *index, const attr_id_t *key_range_begin,
                            const attr_id_t *key_range_end);
grid_cursor_t *vindex_query_append(const struct vindex_t *index, grid_cursor_t *result,
                                   const attr_id_t *key_range_begin, const attr_id_t *key_range_end);
const struct grid_t *vindex_read(grid_cursor_t *result_set);
void vindex_close(grid_cursor_t *result_set);
const attr_id_t *vindex_begin(const vindex_t *index);
const attr_id_t *vindex_end(const vindex_t *index);
void vindex_print(FILE *file, vindex_t *index);
