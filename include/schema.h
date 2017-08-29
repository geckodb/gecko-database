#pragma once

#include <stdinc.h>
#include <containers/vector.h>


typedef struct {
    char *frag_name;
    vector_t *attr;
} schema_t;

schema_t *gs_schema_create(const char *table_name);

schema_t *gs_schema_subset(schema_t *super, const attr_id_t *indicies, size_t nindicies);

void gs_schema_free(schema_t *schema);

schema_t *gs_schema_cpy(const schema_t *schema);

const struct attr_t *gs_schema_attr_by_id(const schema_t *schema, attr_id_t attr_id);

const struct attr_t *gs_schema_attr_by_name(const schema_t *schema, const char *name);

size_t gs_schema_attr_size_by_id(schema_t *schema, attr_id_t attr_id);

size_t gs_schema_num_attributes(const schema_t *schema);

enum field_type gs_schema_attr_type(schema_t *schema, attr_id_t id);