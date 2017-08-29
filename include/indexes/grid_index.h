#pragma once

#include <stdinc.h>
#include <indexes/grid_index.h>

typedef enum {
    GI_VINDEX_HASH,

    GI_HINDEX_BESEARCH
} grid_index_tag;

typedef struct grid_index_result_cursor_t {
    void *extra;
} grid_index_result_cursor_t;

grid_index_result_cursor_t *grid_index_create_cursor(size_t result_capacity);

void grid_index_close(grid_index_result_cursor_t *cursor);

const struct grid_t *grid_index_read(grid_index_result_cursor_t *result_set);
