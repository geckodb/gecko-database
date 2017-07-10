#pragma once

#include <stdinc.h>
#include <attr.h>
#include <containers/vector.h>

typedef struct {
    vector_t *attr;
} SCHEMA;

ATTR *gs_schema_attr_by_id(SCHEMA *frag, ATTR_ID attr_id);