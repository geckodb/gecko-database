#pragma once

#include <stdinc.h>
#include <grid_cursor.h>

typedef enum {
    GI_VINDEX_HASH,

    GI_HINDEX_BESEARCH
} grid_index_tag;

typedef struct grid_cursor_t {
    void *extra;
} grid_cursor_t;

grid_cursor_t *grid_cursor_create(size_t cursor);

void grid_cursor_close(grid_cursor_t *cursor);

void grid_cursor_pushback(grid_cursor_t *cursor, const void *data);

void grid_cursor_append(grid_cursor_t *dst, grid_cursor_t *src);

void grid_cursor_dedup(grid_cursor_t *cursor);

struct grid_t *grid_cursor_next(grid_cursor_t *cursor);

size_t grid_cursor_numelem(const grid_cursor_t *cursor);

bool gs_grid_cursor_is_empty(const grid_cursor_t *cursor);
