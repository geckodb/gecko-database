#pragma once

#include <defs.h>
#include <storage/schema.h>
#include <containers/vector.h>

typedef struct {
    const char *name;
    schema_t *schema;
    vector_t columns;
} thin_column_store_t;

thin_column_store_t *thin_column_store_create(const char *name, schema_t *schema, size_t tuple_capacity);

bool thin_column_store_free(thin_column_store_t *table);

bool thin_column_store_add(thin_column_store_t *table, size_t num_tuples, const void *data);

void thin_column_store_dump(thin_column_store_t *table);