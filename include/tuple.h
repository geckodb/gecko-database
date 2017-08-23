#pragma once

#include <stdinc.h>

struct grid_table_t;

typedef uint32_t tuple_id_t;

typedef struct tuple_t {
    const struct grid_table_t *table;
    tuple_id_t tuple_id;
} tuple_t;

void gs_tuple_open(tuple_t *tuple, const struct grid_table_t *table, tuple_id_t tuple_id);

void gs_tuple_id_init(void *data);

void gs_tuple_id_inc(void *data);
