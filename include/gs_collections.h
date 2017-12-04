#pragma once

#include <gecko-commons/stdinc.h>
#include <gecko-commons/gs_error.h>

typedef struct gs_collection_t gs_collection_t; /* forwarding */

typedef struct gs_collections_t gs_collections_t;

typedef struct collections_result_t {
    gs_status_t status;
    gs_collection_id_t id;
} collections_result_t;

GS_DECLARE(gs_status_t) gs_collections_create(gs_collections_t **collections);

GS_DECLARE(gs_status_t) gs_collections_dispose(gs_collections_t *collections);

GS_DECLARE(gs_status_t) gs_collections_add(collections_result_t **result, gs_collections_t *collections,
                                           const char **name, size_t num_collections);

GS_DECLARE(gs_status_t) gs_collections_result_free(collections_result_t *result);

GS_DECLARE(gs_status_t) gs_collections_by_name(collections_result_t **result, gs_collections_t *collections,
                                               const char **name, size_t num_collections);

GS_DECLARE(gs_status_t) gs_collections_remove(gs_collections_t *collections, gs_collection_id_t id);

GS_DECLARE(gs_status_t) gs_collections_print(FILE *file, const gs_collections_t *collections);

GS_DECLARE(void) gs_collections_id_init(void *data);

GS_DECLARE(void) gs_collections_id_inc(void *data);

