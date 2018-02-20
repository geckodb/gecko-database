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
    slot_t                    **slots;
    read_cache_bulk_fetch_func_t     fetch_func;
    replacement_policy_e        policy;
    void                       *cache_impl;
    read_cache_key_bulk_comp_func_t    key_comp;
    gs_spinlock_t               spinlock;
    slot_t                     *(*get_victim)(read_cache_t *self);
    void                       (*promote_slot)(read_cache_t *self, slot_t *slot);
    void                       *args;
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

typedef enum put_look_for_slot_e
{
    put_look_for_slot_enable, put_look_for_slot_disable
} put_look_for_slot_e;

static inline gs_status_t unsafe_put(read_cache_t *cache, read_cache_counters_t *nullable_counters, const void *keys,
                       const void *values, size_t num_elements, put_look_for_slot_e mode);

void read_cache_impl_lru_promote_slot(read_cache_t *self, slot_t *slot)
{
    // TODO: check that this actually works
    read_cache_impl_lru_t *lru_cache = (read_cache_impl_lru_t*) self->cache_impl;
    lru_slot_t            *lru_slot  = (lru_slot_t *) slot->manager;

    if (lru_slot == lru_cache->last) {
        lru_cache->last = lru_slot->prev;
        lru_cache->last->next = NULL;
    }

    if (lru_slot->prev) {
        lru_slot->prev->next = lru_slot->next;
    }

    if (lru_slot->next) {
        lru_slot->next->prev = lru_slot->prev;
    }



    lru_slot->prev = NULL;
    lru_slot->next = lru_cache->first;
    lru_cache->first->prev = lru_slot;
    lru_cache->first = lru_slot;

}

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

    assert (lru_cache->first != NULL);
    assert (lru_cache->last != NULL);
    assert (lru_cache->first->prev == NULL);
    assert (lru_cache->last->next == NULL);

    return victim->slot;
}

gs_status_t read_cache_create(read_cache_t **cache, size_t key_size, read_cache_key_bulk_comp_func_t key_comp,
                              size_t value_size, size_t capacity, read_cache_bulk_fetch_func_t fetch_func,
                              replacement_policy_e repl_type, void *args)
{
    read_cache_t *result = GS_REQUIRE_MALLOC(sizeof(read_cache_t));
    result->key_size = key_size;
    result->value_size = value_size;
    result->capacity = capacity;
    result->fetch_func = fetch_func;
    result->policy = repl_type;
    result->key_comp = key_comp;
    result->slots = GS_REQUIRE_MALLOC(capacity * sizeof(slot_t *));
    result->args = args;
    gs_spinlock_create(&result->spinlock);

    for (unsigned slot_id = 0; slot_id < capacity; slot_id++)
    {
        slot_t **slot_ptr = result->slots + slot_id;
        *slot_ptr = GS_REQUIRE_MALLOC(sizeof(slot_t));

        **slot_ptr = (slot_t) {
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
            result->cache_impl   = GS_REQUIRE_MALLOC(sizeof(read_cache_impl_lru_t));
            result->get_victim   = read_cache_impl_lru_get_victim;
            result->promote_slot = read_cache_impl_lru_promote_slot;

            read_cache_impl_lru_t *lru_cache = (read_cache_impl_lru_t *) result->cache_impl;
            lru_cache->last = lru_cache->first = NULL;

            for (size_t i = 0; i < capacity; i++) {
                lru_slot_t *lru_slot = GS_REQUIRE_MALLOC(sizeof(lru_slot_t));
                lru_slot->slot = result->slots[i];
                lru_slot->slot->manager = lru_slot;

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
                           size_t *num_not_in_storage, bool **in_storage_mask, void **values, const void *keys,
                           size_t num_keys)
{
    bool     all_contained                  = TRUE;
    int     *comp_result                    = GS_REQUIRE_MALLOC(cache->capacity * sizeof(int));
    void    *read_values                    = GS_REQUIRE_MALLOC(num_keys * cache->value_size);
    bool    *storage_mask                   = GS_REQUIRE_MALLOC(num_keys * sizeof(bool));
    size_t  *keys_not_found_indicies_pass_1 = GS_REQUIRE_MALLOC(num_keys * sizeof(size_t));
    void    *keys_not_found_keys_pass_1     = GS_REQUIRE_MALLOC(num_keys * sizeof(cache->key_size));

    slot_t **slot_list_a;
    slot_t **slot_list_b               = GS_REQUIRE_MALLOC(cache->capacity * sizeof(slot_t*));
    size_t   slot_list_size;
    size_t   slot_list_size_new        = cache->capacity;;

    size_t   num_keys_not_found_pass_1 = 0;

    bool     all_in_storage_total      = TRUE;
    size_t   num_not_in_storage_local  = 0;

    read_cache_counters_t counters;

    /* Protect cache for concurrent access */
    read_cache_lock(cache);

    /* In case statistic information is required, consider the latest state */
    GS_OPTIONAL(nullable_counters != NULL, counters = *nullable_counters);

    // -----------------------------------------------------------------------------------------------------------------
    //
    // PASS 1
    //
    // Search all keys in the slots. Each key is compared in a bulk manner with all slots at once. This
    // pass returns a list of keys from which is known that the values are not in the cache (but maybe in the
    // backing storage) and a mapping of values in both the 'in_storage_mask' (i.e., a flag indicating whether
    // this values is contained in the backing storage) and the 'values' vector (i.e., the concrete values)
    // at the position of key in the input key vector. In total: pass 1 returns values for keys which refer
    // to elements that are stored in the cache (and therefore in the backing storage). For all others
    // (contained or not contained in the backing storage), pass 2 is needed.
    //
    // -----------------------------------------------------------------------------------------------------------------

    /* An optimization in the bulk compare of each key with all slots is applied: All slot pointers are copied to
     * a temporary list 'slot_list'. Whenever a hit of a key happens in 'slot_list', the hit slot is removed from
     * 'slot_list'. The best case is that all keys are stored in the cache. Then, each check removes one slot
     * from 'slot_list'. If 0 <= p <= 1 is the probability that a number m < n of n
     * keys to find are stored in all m slots, then the expected number of iterations for this search is
     * 'p * (sum_{i = 1}^{n} (max(0, (m + 1) - i))) + (1 - p) * (m * n)': if no slot contains a key from the key
     * list then the lookup loops n times over m slots, if on the other hand all slots contain one key from the
     * key list then for each subsequent loop one slot less must be considered which means
     * m + (m - 1) + (m - 2) + ... + 1 slots are considered. */
    for (size_t idx = 0; idx < cache->capacity; idx++)
    {
        slot_list_b[idx] = cache->slots[idx];
    }

    const void *keys_it = keys;

    for (size_t key_idx = 0; key_idx < num_keys; key_idx++)
    {
        const void *key = keys_it;

        /* Mark that entry pessimistically as not contained in the backing storage */
        storage_mask[key_idx] = FALSE;

        /* Swapping the slot lists that are used to reduce the list size of checked slots */
        slot_list_a = slot_list_b;
        slot_list_size = slot_list_size_new;
        slot_list_size_new = 0;

        /* Search all slots at once in order to find a slot that contains the key */
        cache->key_comp(comp_result, key, slot_list_a, slot_list_size);

        /* Write statistics */
        counters.num_slot_compares += slot_list_size;
        counters.num_bulk_key_comp_invoked++;

        /* This search can result in three cases: (1) a slot is found that contains the key, (2) no slot is found
         * that contains the key but the key is stored in the backing storage. Hence, load the value from the
         * backing store and put it into the cache, (3) no slot is found that contains the key and the key itself
         * also not exists in the backing storage */

        bool match_found = FALSE;
        size_t search_list_idx;

        /* Find the slot that contains the key (if any) */
        for (search_list_idx = 0; search_list_idx < slot_list_size; search_list_idx++)
        {
            slot_t *slot = slot_list_a[search_list_idx];

            if (GS_UNLIKELY(comp_result[search_list_idx] == 0))
            {
                /* Case 1: The key is stored in that slot. Remove that slot from the search list by not adding
                 * it into the search list for the next round. Also promote slot */

                memcpy(read_values + key_idx * cache->value_size, slot->value, cache->value_size);

                cache->promote_slot(cache, slot);

                /* That particular key is stored in a slot and hence in the backing storage */
                storage_mask[key_idx] = TRUE;

                /* Stop looking for a match since it was found. Setup the 'match_found' variable in order to
                 * copy eventual remaining slots into the slot search list. Actually, that's a code optimization
                 * in order to skip branching where it is known that the 'else' case now always happens.  */
                match_found = TRUE;

                /* Write statistics */
                counters.read_hit++;

                break;
            }  else {
                /* For that particular key and that particular slot, no match is found. Add this slot to the
                 * search list for the next keys */
                slot_list_b[slot_list_size_new++] = slot;

                /* Write statistics */
                counters.read_miss++;
            }
        }

        /* Update aggregated state whether all keys map to values stored in the backing store. This allows
         * fast determination whether the 'in_storage_mask' must be considered or not. As a code optimization
         * this evaluation runs not in a dedicated loop but in this one. */
        all_contained &= match_found;

        if (match_found)
        {
            /* From the slot search in the previous loop, there might be some slots not added to the search list.
             * These slots are exactly those that where not considered after a key match was found. Since these
             * slots must be considered anyway, add the remaining tail of the old search list to the new one.
             * Actually, that's part of the code optimization from above. */
            for (size_t i = search_list_idx; i < slot_list_size; i++)
            {
                slot_t *slot = slot_list_a[search_list_idx];
                slot_list_b[slot_list_size_new++] = slot;
            }
        }

        /* In case no slot was found that contains the key, store the key_idx in order to search for the entire
         * in the backing storage. Storing the key_idx is an optimization that (1) reduces the number of elements
         * that must be search in the backing storage (i.e., only these not found in the cache), and (2)
         * enables to invoke a bulk operation on the backing storage that avoids to perform a search key by key */
        if (!storage_mask[key_idx])
        {
            keys_not_found_indicies_pass_1[num_keys_not_found_pass_1] = key_idx;
            memcpy(keys_not_found_keys_pass_1 + num_keys_not_found_pass_1 * cache->key_size,
                   key, cache->key_size);
            num_keys_not_found_pass_1++;
        }

        keys_it += cache->key_size;
    }

    // -----------------------------------------------------------------------------------------------------------------
    //
    // PASS 2
    //
    // The first pass yields a list 'keys_not_found_pass_1' that contains the indices of keys in 'keys' that
    // cannot be found in the slots. Pass 2 searches the backing storage for these keys and in case of existence
    // load and store their values in the output list 'values' at that particular indices. Each found value is further
    // added to the cache maybe evicting other values (c.f., replacement strategy)
    //
    // -----------------------------------------------------------------------------------------------------------------

    bool *in_storage_mask_pass_2 = GS_REQUIRE_MALLOC(num_keys_not_found_pass_1 * sizeof(bool));
    bool all_contained_pass_2;
    void *values_pass_2;

    void *cache_put_keys   = GS_REQUIRE_MALLOC(num_keys_not_found_pass_1 * cache->key_size);
    void *cache_put_values = GS_REQUIRE_MALLOC(num_keys_not_found_pass_1 * cache->value_size);
    size_t num_cache_puts  = 0;

    /* Bulk load values by keys from the backing storage (if contained). Whether a particular key refers to a valid
     * value stored in the backing storage is indicated by 'in_storage_mask_pass_2' afterwards */
    cache->fetch_func(&all_contained_pass_2, in_storage_mask_pass_2, &values_pass_2, keys_not_found_keys_pass_1,
                      num_keys_not_found_pass_1, cache->args);

    /* Write statistics */
    counters.num_bulk_fetch_invoked++;

    /* Add values fetched from the backing storage to both the result set and the cache itself */
    for (size_t i = 0; i < num_keys_not_found_pass_1; i++)
    {
        size_t result_idx = keys_not_found_indicies_pass_1[i];

        if (GS_LIKELY(in_storage_mask_pass_2[i]))
        {
            /* In case the key maps to a stored value, store that value in the result set at that position the
             * key was originally stored in the key set */
            const void *key        = keys + result_idx * cache->key_size;
            const void *value      = values_pass_2 + i * cache->value_size;
            memcpy(read_values + result_idx * cache->value_size, value, cache->value_size);

            /* Instead of putting fetched values along with their keys pair-by-pair into the cache, collect
             * them and invoke a bulk cache put operation at the end */
            memcpy(cache_put_keys + num_cache_puts * cache->key_size, key, cache->key_size);
            memcpy(cache_put_values + num_cache_puts * cache->value_size, value, cache->value_size);
            num_cache_puts++;

            /* Write statistics */
            counters.num_found_in_backing_storage++;
        } else
        {
            /* In this case a particular key was given which does not refer to a value stored in the backing
             * storage. Note, this is already marked by 'storage_mask[result_idx] = FALSE' from above */

            /* Further, set the aggregation value 'all_in_storage' such that the caller is notified about at least
             * one key refers to an unknown object */
            all_in_storage_total = FALSE;

            /* Increase the counter for the number of keys not referencing valid objects. This counter might be
             * used by the caller to allocate exactly that amount of memory needed to process missing keys */
            num_not_in_storage_local++;

            /* Write statistics */
            counters.num_not_found_in_backing_storage++;
        }
    }

    /* Invoke a bulk cache put operation to put fetched data into the cache. Disable the functionality in the
     * put function that tries first to find a slot that contains the keys (since it is known at this point
     * that there is no slot with this property */
    unsafe_put(cache, nullable_counters, cache_put_keys, cache_put_values, num_cache_puts, put_look_for_slot_disable);

    *values = read_values;
    *in_storage_mask = storage_mask;
    *all_in_storage = all_in_storage_total;
    *num_not_in_storage = num_not_in_storage_local;

    /* In case statistics are required */
    GS_OPTIONAL(nullable_counters != NULL, *nullable_counters = counters);

    free (comp_result);
   // free (cache_put_keys); // TODO: Memleak?
    free (cache_put_values);
    free (keys_not_found_indicies_pass_1);
    free (in_storage_mask_pass_2);
    free (slot_list_b);


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
    read_cache_lock(cache);
    gs_status_t result = unsafe_put(cache, nullable_counters, keys, values, num_elements, put_look_for_slot_enable);
    read_cache_unlock(cache);
    return result;
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

static inline gs_status_t unsafe_put(read_cache_t *cache, read_cache_counters_t *nullable_counters, const void *keys,
                                     const void *values, size_t num_elements, put_look_for_slot_e mode)
{
    slot_t        *free_slot;

    int           *comp_result = GS_REQUIRE_MALLOC(cache->capacity * sizeof(int));

    read_cache_counters_t counters;

    /* In case statistics are required */
    GS_OPTIONAL(nullable_counters != NULL, counters = *nullable_counters);

    while (num_elements--)
    {
        const void *key = keys;
        const void *value = values;

        // Compare the key with all keys currently stored in the slots
        // TODO: Check whether data parallelism makes sense here

        slot_t *found_slot = NULL;

        if (mode == put_look_for_slot_enable)
        {
            /* Mode indicates that the cache has to search for a slot that contains the key (such that an update
             * can be performed) */

            /* Write statistics */
            counters.num_slot_compares += cache->capacity;
            counters.num_bulk_key_comp_invoked++;

            if (cache->key_comp(comp_result, key, cache->slots, cache->capacity) != GS_SUCCESS) {
                free(comp_result);
                return GS_FAILED;
            }

            // Find the slot which contains the key (if any)
            for (unsigned slot_id = 0; slot_id < cache->capacity; slot_id++) {
                if (GS_UNLIKELY(comp_result[slot_id] == 0)) {
                    found_slot = cache->slots[slot_id];
                    break;
                }
            }

            if (GS_LIKELY(found_slot)) {
                /* There is a slot in the cache that holds the old value */
                /* Perform a value update on this slot */
                found_slot->value = value;

                /* Write statistics */
                counters.num_slot_update_hit++;
            }
        }

        if (mode != put_look_for_slot_enable || !GS_LIKELY(found_slot))
        {
            /* There is no slot in the cache that holds the old value */
            free_slot = cache->get_victim(cache);
            free_slot->flags.in_use = TRUE;
            free_slot->key = key;
            free_slot->value = value;

            /* Write statistics */
            counters.num_evictions++;
        }

        keys += cache->key_size;
        values += cache->value_size;
    }

    /* In case statistics are required */
    GS_OPTIONAL(nullable_counters != NULL, *nullable_counters = counters);

    free (comp_result);
    return GS_FAILED;
}