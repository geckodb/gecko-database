#pragma once

#include <defs.h>

struct attribute;

enum data_type {
    DT_UNDEFINED,
    DT_BOOLEAN,
    DT_UNSIGNED_BYTE,
    DT_SIGNED_BYTE,
    DT_UNSIGNED_SHORT,
    DT_SIGNED_SHORT,
    DT_UNSIGNED_INT,
    DT_SIGNED_INT,
    DT_UNSIGNED_LONG,
    DT_SIGNED_LONG,
    DT_FLOAT,
    DT_DOUBLE,
    DT_STRING,
    DT_INTERNAL_TID
};

struct attribute *attribute_create(enum data_type type, const char *name);

bool attribute_delete(struct attribute *attr);

enum data_type attribute_get_type(const struct attribute *attr);

const char *attribute_get_name(const struct attribute *attr);

bool attribute_equals(const void *a, const void *b);

bool check_attribute_non_null(const struct attribute *attr);

struct attribute *attribute_copy(const struct attribute *attr);

size_t attribute_get_sizeof_ptr();