#pragma once

#include <stdinc.h>
#include <tuple.h>
#include <tuplet_field.h>
#include <grid.h>

typedef struct tuple_field_t {
    tuple_t *tuple;
    attr_id_t table_attr_id;

    const grid_t *grid;
    tuplet_t *tuplet;
    tuplet_field_t *field;
    attr_id_t frag_attr_id;
} tuple_field_t;

void gs_tuple_field_open(tuple_field_t *field, tuple_t *tuple);

void gs_tuple_field_seek(tuple_field_t *field, tuple_t *tuple, attr_id_t attr_id);

void gs_tuple_field_next(tuple_field_t *field);

void gs_tuple_field_write(tuple_field_t *field, const void *data);

const void *gs_tuple_field_read(tuple_field_t *field);

void gs_tuple_field_close(tuple_field_t *field);

/*bool gs_tuple_field_next(tuple_field_t *field);

const void *gs_tuple_field_read(tuple_field_t *field);

void gs_tuple_field_update(tuple_field_t *field, const void *data);

bool gs_tuple_field_write(tuple_field_t *field, const void *data);

void gs_tuple_field_set_null(tuple_field_t *field);

bool gs_tuple_field_is_null(tuple_field_t *field);

void gs_tuple_field_close(tuple_field_t *field);*/