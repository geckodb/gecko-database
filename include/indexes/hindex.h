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
#include <interval.h>
#include <grid_cursor.h>
#include <schema.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct grid_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    HT_LINEAR_SEARCH
} hindex_tag;

typedef struct hindex_result_t {
    hindex_tag tag;
    void *extra;
} xxx_index_result_cursor_t;

typedef struct hindex_t {
    hindex_tag tag;
    tuple_id_interval_t bounds;

    void (*_add)(struct hindex_t *self, const tuple_id_interval_t *key, const struct grid_t *grid);
    void (*_remove_interval)(struct hindex_t *self, const tuple_id_interval_t *key);
    void (*_remove_intersec)(struct hindex_t *self, tuple_id_t tid);
    bool (*_contains)(const struct hindex_t *self, tuple_id_t tid);
    void (*_query)(grid_cursor_t *result, const struct hindex_t *self, const tuple_id_t *tid_begin,
                   const tuple_id_t *tid_end);
    void (*_delete)(struct hindex_t *self);
    tuple_id_t (*_minbegin)(struct hindex_t *self);
    tuple_id_t (*_maxend)(struct hindex_t *self);

    const schema_t *table_schema; /*<! Required to calculate an approx. result set size for queries */
    void *extra;
} hindex_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void hindex_delete(struct hindex_t *index);
void hindex_add(struct hindex_t *index, const tuple_id_interval_t *key, const struct grid_t *grid);
void hindex_remove(struct hindex_t *index, const tuple_id_interval_t *key);
void hindex_remove_having(struct hindex_t *index, tuple_id_t tid);
bool hindex_contains(const struct hindex_t *index, tuple_id_t tid);
grid_cursor_t *hindex_query(const struct hindex_t *index, const tuple_id_t *tid_begin,
                            const tuple_id_t *tid_end);
const struct grid_t *hindex_read(grid_cursor_t *result_set);
void hindex_close(grid_cursor_t *result_set);
void hindex_bounds(tuple_id_interval_t *bounds, const hindex_t *index);
void hindex_print(FILE *file, const hindex_t *index);
