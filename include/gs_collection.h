#pragma once

#include <gs.h>

typedef struct gs_collection_t gs_collection_t;

GS_DECLARE(gs_status_t) gs_collection_create(gs_collection_t **collection, const char *name);

GS_DECLARE(gs_status_t) gs_collection_dispose(gs_collection_t *collection);

GS_DECLARE(const char*) gs_collection_get_name(gs_collection_t *collection);

GS_DECLARE(gs_status_t) gs_collection_print(FILE *file, gs_collection_t *collection);