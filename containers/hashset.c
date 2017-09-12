#include <containers/hashset.h>
#include <containers/dicts/hash_table.h>

void gs_hashset_create(hashset_t *out, size_t elem_size, size_t capacity)
{
    REQUIRE_NONNULL(out)
    REQUIRE_NONZERO(elem_size)
    REQUIRE_NONZERO(capacity)
    out->dict = hash_table_create_jenkins(elem_size, sizeof(bool), capacity, 1.7f, 0.75f);
    out->vec  = vector_create(sizeof(elem_size), capacity);
}

void gs_hashset_free(hashset_t *set)
{
    REQUIRE_NONNULL(set)
    dict_free(set->dict);
    vector_free(set->vec);
    set->dict = NULL;
    set->vec = NULL;
}

void gs_hashset_add(hashset_t *set, const void *data, size_t num_elems)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->vec)
    REQUIRE_NONNULL(set->dict)
    REQUIRE_NONNULL(data)
    REQUIRE_NONZERO(num_elems)
    if (!dict_contains_key(set->dict, data)) {
        vector_add(set->vec, num_elems, data);
        bool dummy;
        dict_put(set->dict, data, &dummy);
    }
}

void gs_hashset_remove(hashset_t *set, const void *data, size_t num_elems)
{
    panic("Not implemented: '%s'. Hashtable remove is not implemented correctly currently", "gs_hashset_remove");
}

bool gs_hashset_contains(const hashset_t *set, const void *data)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->dict)
    return dict_contains_key(set->dict, data);
}

const void *gs_hashset_begin(const hashset_t *set)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->vec)
    return vector_begin(set->vec);
}

const void *gs_hashset_end(const hashset_t *set)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->vec)
    return vector_end(set->vec);
}