#pragma once

#include <defs.h>
#include <storage/attribute.h>

struct schema
{
    struct vector *attributes;
};

struct schema *schema_create(size_t attribute_capacity);

bool schema_delete(struct schema *s);

bool schema_equals(struct schema *lhs, struct schema *rhs);

size_t schema_get_num_of_attributes(struct schema *s);

bool schema_set_attribute(struct schema *s, size_t attribute_idx, const struct attribute *attr);

enum data_type schema_get_attribute_type(struct schema *s, size_t attribute_idx);

const char *schema_get_attribute_name(struct schema *s, size_t attribute_idx);