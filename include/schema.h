#pragma once

#include <stdinc.h>
#include <containers/vector.h>


typedef struct {
    char *table_name;
    vector_t *attr;
    vector_t *mapped_attr_idx;
} schema_t;

schema_t *gs_schema_create(const char *table_name);

schema_t *gs_schema_subset(schema_t *super, vector_t /*of size_t*/ *indices);

void gs_schema_free(schema_t *schema);

schema_t *gs_schema_cpy(schema_t *schema);

const struct attr_t *gs_schema_attr_by_id(const schema_t *schema, attr_id_t attr_id);

size_t gs_schema_attr_size_by_id(schema_t *schema, attr_id_t attr_id);

size_t gs_schema_num_attributes(schema_t *schema);

enum field_type gs_schema_attr_type(schema_t *schema, attr_id_t id);