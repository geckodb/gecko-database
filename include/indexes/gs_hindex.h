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
#include <gecko-commons/gs_interval.h>

#include <gs_grid_cursor.h>
#include <gs_schema.h>
#include <gs_interval.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct gs_grid_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    HT_LINEAR_SEARCH
} gs_hindex_tag_e;

typedef struct gs_hindex_result_t {
    gs_hindex_tag_e tag;
    void *extra;
} gs_xxx_index_result_cursor_t;

typedef struct gs_hindex_t {
    gs_hindex_tag_e tag;
    gs_tuple_id_interval_t bounds;

    void (*_add)(struct gs_hindex_t *self, const gs_tuple_id_interval_t *key, const struct gs_grid_t *grid);
    void (*_remove_interval)(struct gs_hindex_t *self, const gs_tuple_id_interval_t *key);
    void (*_remove_intersec)(struct gs_hindex_t *self, gs_tuple_id_t tid);
    bool (*_contains)(const struct gs_hindex_t *self, gs_tuple_id_t tid);
    void (*_query)(gs_grid_cursor_t *result, const struct gs_hindex_t *self, const gs_tuple_id_t *tid_begin,
                   const gs_tuple_id_t *tid_end);
    void (*_delete)(struct gs_hindex_t *self);
    gs_tuple_id_t (*_minbegin)(struct gs_hindex_t *self);
    gs_tuple_id_t (*_maxend)(struct gs_hindex_t *self);

    const gs_schema_t *table_schema; /*<! Required to calculate an approx. result set size for queries */
    void *extra;
} gs_hindex_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void gs_hindex_delete(struct gs_hindex_t *index);
void gs_hindex_add(struct gs_hindex_t *index, const gs_tuple_id_interval_t *key, const struct gs_grid_t *grid);
void gs_hindex_remove(struct gs_hindex_t *index, const gs_tuple_id_interval_t *key);
void gs_hindex_remove_having(struct gs_hindex_t *index, gs_tuple_id_t tid);
bool gs_hindex_contains(const struct gs_hindex_t *index, gs_tuple_id_t tid);
gs_grid_cursor_t *gs_hindex_query(const struct gs_hindex_t *index, const gs_tuple_id_t *tid_begin,
                                  const gs_tuple_id_t *tid_end);
const struct gs_grid_t *gs_hindex_read(gs_grid_cursor_t *result_set);
void gs_hindex_close(gs_grid_cursor_t *result_set);
void gs_hindex_bounds(gs_tuple_id_interval_t *bounds, const gs_hindex_t *index);
void gs_hindex_print(FILE *file, const gs_hindex_t *index);
