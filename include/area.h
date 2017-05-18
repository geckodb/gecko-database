#pragma once

#include <defs.h>
#include <containers/vector.h>

typedef struct {

} mdb_table_area;


mdb_table_area *mdb_table_area_all(const mdb_table_space_context *context);

mdb_table_area *mdb_table_area_alloc(mdb_area_context *context, const char *area_name);

mdb_table_area *mdb_table_by_name(mdb_area_context *context, const char *area_name);

bool mdb_table_area_free(mdb_table_area *space);