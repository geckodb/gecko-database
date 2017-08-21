#pragma once

#include <stdinc.h>
#include <tuple.h>

typedef struct tuple_field_t {

} tuple_field_t;

tuple_field_t *gs_tuple_field_open(tuple_t *tuple);

bool gs_tuple_field_next(tuple_field_t *field);

const void *gs_tuple_field_read(tuple_field_t *field);

void gs_tuple_field_update(tuple_field_t *field, const void *data);

bool gs_tuple_field_write(tuple_field_t *field, const void *data);

void gs_tuple_field_set_null(tuple_field_t *field);

bool gs_tuple_field_is_null(tuple_field_t *field);

void gs_tuple_field_close(tuple_field_t *field);