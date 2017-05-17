#pragma once

#include <defs.h>
#include <containers/vector.h>

typedef struct {

} mdb_table_space_context;

typedef struct {

} mdb_table_space;

extern mdb_vector *__global_table_space_context_register;

mdb_table_space_context *mdb_table_space_context_alloc(const char *context_name);

bool mdb_table_space_context_free(const char *context_name);

mdb_table_space *mdb_table_space_by_name(const mdb_table_space_context *context, const char *name);

mdb_table_space *mdb_table_space_all(const mdb_table_space_context *context);

mdb_table_space *mdb_table_space_alloc(mdb_table_space_context *context, const char *space_name);

bool mdb_table_space_free(mdb_table_space *space);