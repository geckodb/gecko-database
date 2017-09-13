#pragma once

#include <stdinc.h>
#include <tuple.h>
#include <tuplet_field.h>
#include <grid.h>

typedef struct tuple_field_t
{
    const grid_t *grid;
    attr_id_t table_attr_id;
    attr_id_t grid_attr_id;
    tuple_t *tuple;
    tuplet_t tuplet;
    tuplet_field_t tuplet_field;
} tuple_field_t;

void gs_tuple_field_open(tuple_field_t *field, tuple_t *tuple);

void gs_tuple_field_seek(tuple_field_t *tuple_field, tuple_t *tuple, attr_id_t table_attr_id);

void gs_tuple_field_next(tuple_field_t *field);

void gs_tuple_field_write(tuple_field_t *field, const void *data);

const void *gs_tuple_field_read(tuple_field_t *field);

/*bool gs_tuple_field_next(tuple_field_t *tuplet_field);

const void *gs_tuple_field_read(tuple_field_t *tuplet_field);

void gs_tuple_field_update(tuple_field_t *tuplet_field, const void *data);

bool gs_tuple_field_write(tuple_field_t *tuplet_field, const void *data);

void gs_tuple_field_set_null(tuple_field_t *tuplet_field);

bool gs_tuple_field_is_null(tuple_field_t *tuplet_field);

void gs_tuple_field_close(tuple_field_t *tuplet_field);*/