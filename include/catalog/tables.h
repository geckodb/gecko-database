#pragma once

typedef struct
{

} mdb_table_context;

typedef struct
{
    mdb_table_context *context;

} mdb_table_handle;

typedef struct
{

} mdb_table;

enum mdb_table_type
{
    TT_PLAIN_ROWSTORE
};

mdb_table_context *mdb_table_context_

mdb_table_handle *mdb_table_create(mdb_table_catalog *context, enum mdb_table_type type, const char *name, const mdb_schema *schema, size_t init_capacity);