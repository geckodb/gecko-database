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

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct grid_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    GIT_HASH
} grid_index_tag;

typedef struct index_query_t {
    grid_index_tag tag;
    void *extra;
} index_query_t;

typedef struct grid_index_t {
    grid_index_tag tag;

    void (*_add)(struct grid_index_t *self, size_t key, const struct grid_t *grid);
    void (*_remove)(struct grid_index_t *self, size_t key);
    bool (*_contains)(const struct grid_index_t *self, size_t key);
    index_query_t *(*_query_open)(const struct grid_index_t *self, size_t key_range_begin, size_t key_range_end);
    index_query_t *(*_query_append)(const struct grid_index_t *self, index_query_t *result, size_t key_range_begin, size_t key_range_end);
    const struct grid_t *(*_query_read)(const struct grid_index_t *self, index_query_t *result_set);
    void (*_query_close)(index_query_t *result_set);
    void (*_free)(struct grid_index_t *self);

    void *extra;
} grid_index_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void grid_index_free(grid_index_t *index);

void grid_index_add(grid_index_t *index, size_t key, const struct grid_t *grid);
void grid_index_remove(grid_index_t *index, size_t key);
bool grid_index_contains(const grid_index_t *index, size_t key);
index_query_t *grid_index_query_open(const struct grid_index_t *index, size_t key_range_begin, size_t key_range_end);
index_query_t *grid_index_query_append(const struct grid_index_t *index, index_query_t *result, size_t key_range_begin, size_t key_range_end);
const struct grid_t *grid_index_query_read(const struct grid_index_t *index, index_query_t *result_set);
void grid_index_query_close(const struct grid_index_t *index, index_query_t *result_set);
