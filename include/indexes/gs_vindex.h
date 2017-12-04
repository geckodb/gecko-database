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
#include <gecko-commons/containers/gs_hashset.h>

#include <gs_grid_cursor.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct gs_grid_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_vindex_t {
    gs_grid_index_tag_e tag;
    gs_hashset_t keys;

    void (*_add)(struct gs_vindex_t *self, const gs_attr_id_t *key, const struct gs_grid_t *grid);
    void (*_remove)(struct gs_vindex_t *self, const gs_attr_id_t *key);
    bool (*_contains)(const struct gs_vindex_t *self, const gs_attr_id_t *key);
    void (*_query)(gs_grid_cursor_t *result, const struct gs_vindex_t *self, const gs_attr_id_t *key_begin,
                                  const gs_attr_id_t *key_end);
    void (*_free)(struct gs_vindex_t *self);

    void *extra;
} gs_vindex_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void gs_vindex_delete(gs_vindex_t *index);
void gs_vindex_add(gs_vindex_t *index, const gs_attr_id_t *key, const struct gs_grid_t *grid);
void gs_vindex_remove(gs_vindex_t *index, const gs_attr_id_t *key);
bool gs_vindex_contains(const gs_vindex_t *index, const gs_attr_id_t *key);
gs_grid_cursor_t *gs_vindex_query(const struct gs_vindex_t *index, const gs_attr_id_t *key_range_begin,
                                  const gs_attr_id_t *key_range_end);
gs_grid_cursor_t *gs_vindex_query_append(const struct gs_vindex_t *index, gs_grid_cursor_t *result,
                                         const gs_attr_id_t *key_range_begin, const gs_attr_id_t *key_range_end);
const struct gs_grid_t *gs_vindex_read(gs_grid_cursor_t *result_set);
void gs_vindex_close(gs_grid_cursor_t *result_set);
const gs_attr_id_t *gs_vindex_begin(const gs_vindex_t *index);
const gs_attr_id_t *gs_vindex_end(const gs_vindex_t *index);
void gs_vindex_print(FILE *file, gs_vindex_t *index);
