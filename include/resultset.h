#pragma once

#include <tuple.h>

typedef struct resultset_t {
    struct grid_table_t *context;
    tuple_id_t *tuple_ids;
    size_t ntuple_ids;
    size_t tuple_id_cursor;
} resultset_t;

void gs_resultset_create(resultset_t *resultset, struct grid_table_t *context, tuple_id_t *tuple_ids, size_t ntuple_ids);

void gs_resultset_free(resultset_t *resultset);

void gs_resultset_rewind(resultset_t *resultset);

bool gs_resultset_next(tuple_t *tuple, resultset_t *resultset);
