#include <storage/table.h>
#include <require.h>
#include <assert.h>
#include <storage/schema.h>
#include <storage/attribute.h>

static bool _alloc_data(thin_column_store_t *);

thin_column_store_t *thin_column_store_create(const char *name, schema_t *schema, size_t tuple_capacity)
{
    thin_column_store_t *table = NULL;
    if (require_non_null(name) && require_non_null(schema) && ((table = require_malloc(sizeof(thin_column_store_t))))) {
        table->name = strdup(name);
        table->schema = schema_cpy(schema);
        _alloc_data(table);
    }
    return table;
}

bool thin_column_store_free(thin_column_store_t *table)
{
    return true;
}

bool thin_column_store_add(thin_column_store_t *table, size_t num_tuples, const void *data)
{
    return true;
}

void thin_column_store_dump(thin_column_store_t *table)
{

}

bool _alloc_data(thin_column_store_t *table)
{
    assert (table != NULL);
    size_t num_attributes = schema_num_attributes(table->schema);
    for (size_t attr_idx = 0; attr_idx < num_attributes; attr_idx++) {
     //   data_type type = schema_get(table->schema, attr_idx);
        //   size_t type_size = type_sizeof(type);
    }
    return true;
}