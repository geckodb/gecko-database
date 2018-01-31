#include <storage/buffer.h>
#include <gecko-commons/gecko-commons.h>

#define CACHE_GET_PROPERTY(cache, propname)      \
{                                                \
    if (GS_LIKELY(cache)) {                      \
        *propname = cache->propname;             \
        return GS_SUCCESS;                       \
    } else return GS_FAILED;                     \
}

typedef struct lru_slot_t lru_slot_t;

typedef struct read_cache_t read_cache_t;

typedef struct read_cache_t {
    size_t                      key_size;
    size_t                      value_size;
    size_t                      capacity;
    slot_t                     *slots;
    read_cache_fetch_func_t     fetch_func;
    replacement_policy_e        policy;
    void                       *cache_impl;
    read_cache_key_bulk_comp    key_comp;
    gs_spinlock_t               spinlock;
    slot_t                     *(*get_victim)(read_cache_t *self);
} read_cache_t;

typedef struct lru_slot_t
{
    slot_t                    *slot;
    lru_slot_t                *prev, *next;
} lru_slot_t;

typedef struct read_cache_impl_lru_t
{
    lru_slot_t                *first;
    lru_slot_t                *last;
} read_cache_impl_lru_t;

slot_t *read_cache_impl_lru_get_victim(read_cache_t *self)
{
    read_cache_impl_lru_t *lru_cache = (read_cache_impl_lru_t*) self->cache_impl;

    /* The the last (oldest) slot and put it to the front (newest one) assuming the caller of the victim function
     * will modify the slots content */
    lru_slot_t *victim = lru_cache->last;
    victim->slot->last_mod = gs_timer_now();

    /* The case where the cache contains only one slot is not implemented */
    assert (lru_cache->first != lru_cache->last);

    lru_cache->last        = lru_cache->last->prev;
    lru_cache->last->next  = NULL;
    victim->prev            = NULL;
    victim->next            = lru_cache->first;
    lru_cache->first->prev = victim;
    lru_cache->first       = victim;

    return victim->slot;
}

gs_status_t read_cache_create(read_cache_t **cache, size_t key_size, read_cache_key_bulk_comp key_comp,
                              size_t value_size, size_t capacity, read_cache_fetch_func_t fetch_func,
                              replacement_policy_e repl_type)
{
    read_cache_t *result = GS_REQUIRE_MALLOC(sizeof(read_cache_t));
    result->key_size = key_size;
    result->value_size = value_size;
    result->capacity = capacity;
    result->fetch_func = fetch_func;
    result->policy = repl_type;
    result->key_comp = key_comp;
    result->slots = GS_REQUIRE_MALLOC(capacity * sizeof(slot_t));
    gs_spinlock_create(&result->spinlock);

    for (unsigned slot_id = 0; slot_id < capacity; slot_id++)
    {
        slot_t *slot = result->slots + slot_id;

        *slot = (slot_t) {
                .slot_id                    = slot_id,
                .flags.in_use               = FALSE,
                .last_mod                   = gs_timer_now(),
                .last_read                  = gs_timer_now(),
                .num_reads_since_fetch      = 0,
                .num_reads_total            = 0,
        };
    }

    switch (repl_type) {
        case replacement_policy_lru: {
            result->cache_impl = GS_REQUIRE_MALLOC(sizeof(read_cache_impl_lru_t));
            result->get_victim  = read_cache_impl_lru_get_victim;

            read_cache_impl_lru_t *lru_cache = (read_cache_impl_lru_t *) result->cache_impl;
            lru_cache->last = lru_cache->first = NULL;

            while (capacity--) {
                lru_slot_t *lru_slot = GS_REQUIRE_MALLOC(sizeof(lru_slot_t));
                lru_slot->slot = result->slots++;

                if (GS_LIKELY(lru_cache->first != NULL)) {
                    lru_slot->next = lru_cache->first;
                    lru_cache->first = lru_slot;
                    lru_slot->next->prev = lru_slot;
                } else {
                    lru_cache->first = lru_cache->last = lru_slot;
                }
            }
        } break;
        default: return GS_INTERNALERR;
    }

    *cache = result;

    return GS_SUCCESS;
}

gs_status_t read_cache_dispose(read_cache_t *cache)
{
    return GS_FAILED;
}

gs_status_t read_cache_get(read_cache_t *cache, read_cache_counters_t *nullable_counters, bool *all_in_storage,
                           size_t *num_not_in_storage, bool *in_storage_mask, void **values, const void *key,
                           size_t num_elements)
{
    read_cache_lock(cache);


    read_cache_unlock(cache);
    return GS_FAILED;
}

gs_status_t read_cache_lock(read_cache_t *cache)
{
    return gs_spinlock_lock(&cache->spinlock);
}

gs_status_t read_cache_unlock(read_cache_t *cache)
{
    return gs_spinlock_unlock(&cache->spinlock);
}

gs_status_t read_cache_put(read_cache_t *cache, read_cache_counters_t *nullable_counters, const void *keys,
                           const void *values,
                           size_t num_elements)
{
    slot_t        *free_slot;

    int           *comp_result = GS_REQUIRE_MALLOC(cache->capacity * sizeof(int));

    read_cache_lock(cache);

    while (num_elements--)
    {
        const void *key = keys;
        const void *value = values;

        // Compare the key with all keys currently stored in the slots
        // TODO: Check whether data parallelism makes sense here
        if (cache->key_comp(comp_result, key, cache->slots, cache->capacity) != GS_SUCCESS) {
            free (comp_result);
            read_cache_unlock(cache);
            return GS_FAILED;
        }

        // Find the slot which contains the key (if any)
        slot_t *found_slot = NULL;
        for (unsigned slot_id = 0; slot_id < cache->capacity; slot_id++) {
            if (GS_UNLIKELY(comp_result[slot_id] == 0)) {
                found_slot = cache->slots + slot_id;
                break;
            }
        }

        if (GS_LIKELY(found_slot)) {
            /* There is a slot in the cache that holds the old value */
            /* Perform a value update on this slot */
            found_slot->value = value;
        } else {
            /* There is no slot in the cache that holds the old value */
            free_slot = cache->get_victim(cache);
            free_slot->flags.in_use = TRUE;
            free_slot->key = key;
            free_slot->value = value;
        }

        keys += cache->key_size;
        values += cache->value_size;
    }

    read_cache_unlock(cache);
    free (comp_result);
    return GS_FAILED;
}

gs_status_t read_cache_get_key_size(size_t *key_size, const read_cache_t *cache)
{
    CACHE_GET_PROPERTY(cache, key_size)
}

gs_status_t read_cache_get_value_size(size_t *value_size, const read_cache_t *cache)
{
    CACHE_GET_PROPERTY(cache, value_size)
}

gs_status_t read_cache_get_capcity(size_t *capacity, const read_cache_t *cache)
{
    CACHE_GET_PROPERTY(cache, capacity)
}

gs_status_t read_cache_get_policy(replacement_policy_e *policy, const read_cache_t *cache)
{
    CACHE_GET_PROPERTY(cache, policy)
}
