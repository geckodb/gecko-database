#pragma once

#include <stdinc.h>
#include <attr.h>
#include <containers/vector.h>

typedef struct {
    vector_t *attr;
} schema_t;

attr_t *gs_schema_attr_by_id(schema_t *frag, attr_id_t attr_id);

size_t gs_schema_attr_size_by_id(schema_t *schema, attr_id_t attr_id);