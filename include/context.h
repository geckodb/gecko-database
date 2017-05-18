#pragma once

typedef struct
{

} mdb_area_context;

mdb_area_context *mdb_area_context_alloc();

mdb_area_context *mdb_area_context_free(mdb_area_context *context);

bool mdb_area_context_gc(mdb_area_context *context);

