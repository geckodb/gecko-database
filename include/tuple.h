#pragma once

#include <stdinc.h>

struct grid_table;

typedef uint32_t tuple_id_t;

typedef struct tuple_t {

} tuple_t;

tuple_t *gs_tuple_open(struct grid_table *table, tuple_id_t tuple_id);

void gs_tuple_close(tuple_t *tuple);

bool gs_tuple_next(tuple_t *tuple);

tuple_t *gs_tuple_rewind(tuple_t *tuple);

void gs_tuple_set_null(tuple_t *tuple);

bool gs_tuple_is_null(tuple_t *tuple);

size_t gs_tuple_size(tuple_t *tuple);