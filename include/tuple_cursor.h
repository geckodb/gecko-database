#pragma once

#include <tuple.h>

typedef struct tuple_cursor_t {
    struct grid_table_t *context;
    tuple_id_t *tuple_ids;
    size_t ntuple_ids;
    size_t tuple_id_cursor;
} tuple_cursor_t;

void gs_tuple_cursor_create(tuple_cursor_t *resultset, struct grid_table_t *context, tuple_id_t *tuple_ids,
                            size_t ntuple_ids);

void gs_tuple_cursor_free(tuple_cursor_t *resultset);

void gs_tuple_cursor_rewind(tuple_cursor_t *resultset);

bool gs_tuple_cursor_next(tuple_t *tuple, tuple_cursor_t *resultset);
