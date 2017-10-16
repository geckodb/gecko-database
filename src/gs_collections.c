#include <gs_collections.h>
#include <containers/vec.h>
#include <containers/freelist.h>
#include <gs_collection.h>

typedef struct gs_collections_t
{
    vec_t              *collections;
    freelist_t          id_freelist;
} gs_collections_t;

typedef struct gs_collections_entry_t
{
    gs_collection_t    *collection;
    bool                in_use;
} gs_collections_entry_t;

bool free_collection_object(void *capture, void *begin, void *end);
bool name_is_unique(gs_collections_t *collections, const char *name);
collection_id_t find_by_name(gs_collections_t *collections, const char *name);

GS_DECLARE(gs_status_t) gs_collections_create(gs_collections_t **collections)
{
    gs_collections_t *result = GS_REQUIRE_MALLOC(sizeof(gs_collections_t));
    result->collections = vec_new(sizeof(gs_collections_entry_t), 1);
    freelist_create(&result->id_freelist, sizeof(collection_id_t), 10, gs_collections_id_init, gs_collections_id_inc);
    *collections = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_collections_dispose(gs_collections_t *collections)
{
    GS_REQUIRE_NONNULL(collections);
    vec_free_ex(collections->collections, NULL, free_collection_object);
    freelist_dispose(&collections->id_freelist);
    free(collections);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_collections_add(collections_result_t **result, gs_collections_t *collections,
                                           const char **name, size_t num_collections)
{
    GS_REQUIRE_NONNULL(result);
    GS_REQUIRE_NONNULL(collections);
    GS_REQUIRE_NONNULL(name);
    collections_result_t *retval = GS_REQUIRE_MALLOC(num_collections * sizeof(collections_result_t));
    *result = retval;

    while (num_collections--) {
        if (name_is_unique(collections, *name)) {
            freelist_bind(&retval->id, &collections->id_freelist, 1);

            gs_collections_entry_t entry = {
                .in_use = true,
            };

            gs_collection_create(&entry.collection, *name);
            vec_set(collections->collections, retval->id, 1, &entry);
            retval->status = GS_SUCCESS;
        } else {
            retval->status = GS_REJECTED;
        }
        name++;
        retval++;
    }

    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_collections_result_free(collections_result_t *result)
{
    GS_REQUIRE_NONNULL(result);
    free (result);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_collections_by_name(collections_result_t **result, gs_collections_t *collections,
                                               const char **name, size_t num_collections)
{
    GS_REQUIRE_NONNULL(result);
    GS_REQUIRE_NONNULL(collections);
    GS_REQUIRE_NONNULL(name);

    collections_result_t *retval = GS_REQUIRE_MALLOC(num_collections * sizeof(collections_result_t));
    *result = retval;

    while (num_collections--) {
        if ((retval->id = find_by_name(collections, *name)) != COLLECTION_ID_NULL) {
            retval->status = GS_SUCCESS;
        } else {
            retval->status = GS_NOTFOUND;
        }
        retval++;
        name++;
    }

    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_collections_remove(gs_collections_t *collections, collection_id_t id)
{
    GS_REQUIRE_NONNULL(collections);
    if (id < vec_length(collections->collections)) {
        gs_collections_entry_t *entry = *(gs_collections_entry_t **)vec_at(collections->collections, id);
        if (entry->in_use) {
            entry->in_use = false;
            freelist_pushback(&collections->id_freelist, 1, &id);
        }
    }
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_collections_print(FILE *file, const gs_collections_t *collections)
{
    GS_REQUIRE_NONNULL(file);
    GS_REQUIRE_NONNULL(collections);
    gs_collections_entry_t *begin = vec_begin(collections->collections);
    gs_collections_entry_t *end = vec_end(collections->collections);

    fprintf(file, "%s :)\n", "Yeah");

    for (gs_collections_entry_t *it = begin; it != end; it++) {
        if (it->in_use) {
            gs_collection_print(file, it->collection);
        }
    }
    return GS_SUCCESS;
}

GS_DECLARE(void) gs_collections_id_init(void *data)
{
    *((collection_id_t *) data) = 0;
}

GS_DECLARE(void) gs_collections_id_inc(void *data)
{
    *((collection_id_t *) data) += 1;
}

bool free_collection_object(void *capture, void *begin, void *end)
{
    for (gs_collections_entry_t *it = (gs_collections_entry_t *) begin; it != end; it++) {
        if (it->in_use) {
            gs_collection_dispose(it->collection);
        }
    }
    return GS_SUCCESS;
}

bool name_is_unique(gs_collections_t *collections, const char *name)
{
    return (find_by_name(collections, name) != COLLECTION_ID_NULL);
}

collection_id_t find_by_name(gs_collections_t *collections, const char *name)
{
    gs_collections_entry_t *begin = vec_begin(collections->collections);
    gs_collections_entry_t *end = vec_end(collections->collections);
    for (gs_collections_entry_t *it = begin; it != end; it++) {
        if (it->in_use && strcmp(gs_collection_get_name(it->collection), name) == 0)
            return (it - begin);
    }
    return COLLECTION_ID_NULL;
}