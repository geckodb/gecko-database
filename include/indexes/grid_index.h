#pragma once

#include <stdinc.h>
#include <indexes/grid_index.h>

typedef enum {
    GI_VINDEX_HASH,

    GI_HINDEX_BESEARCH
} grid_index_tag;

typedef struct grid_index_result_cursor_t {
    void *extra;
} grid_set_cursor_t;

grid_set_cursor_t *grid_set_cursor_create(size_t result_capacity);

void grid_set_cursor_close(grid_set_cursor_t *cursor);

void grid_set_cursor_pushback(grid_set_cursor_t *cursor, const void *data);

const struct grid_t *grid_set_cursor_next(grid_set_cursor_t *result_set);

size_t grid_set_cursor_numelem(const grid_set_cursor_t *result_set);
