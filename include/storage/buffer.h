#include <gecko-commons/gs_error.h>
#include <gecko-commons/gs_timer.h>

#ifndef GECKO_READ_CACHE_H
#define GECKO_READ_CACHE_H

typedef struct read_cache_t read_cache_t;

typedef struct slot_t {
    unsigned                  slot_id;
    const void                *key;
    const void                *value;
    struct {
        short                 in_use   : 1;
    } flags;
    size_t                    num_reads_since_fetch;
    size_t                    num_reads_total;
    timestamp_t               last_read;
    timestamp_t               last_mod;
    void                     *manager;
} slot_t;

typedef struct read_cache_counters_t
{
    unsigned num_slot_compares;
    unsigned num_bulk_fetch_invoked;
    unsigned num_bulk_key_comp_invoked;
    unsigned num_found_in_backing_storage;
    unsigned num_not_found_in_backing_storage;

    unsigned read_miss;
    unsigned read_hit;

    unsigned num_evictions;
    unsigned num_slot_update_hit;
} read_cache_counters_t;

typedef enum replacement_policy_e
{
    /* last recently used */
    replacement_policy_lru
} replacement_policy_e;

typedef gs_status_t (*read_cache_bulk_fetch_func_t)(bool *all_contained, bool *in_storage_mask, void **values,
                                                    const void *keys, size_t num_keys, void *args);

/* compares 'needle' with all 'nhaystack' elements in 'haystack' and stores these 'nhaystack' comparison results
 * in 'result' */
typedef gs_status_t (*read_cache_key_bulk_comp_func_t)(int *result, const void *needle, slot_t **haystack,
                                                       size_t nhaystack);

gs_status_t read_cache_create(read_cache_t **cache, size_t key_size, read_cache_key_bulk_comp_func_t key_comp,
                              size_t value_size, size_t capacity, read_cache_bulk_fetch_func_t fetch_func,
                              replacement_policy_e repl_type, void *args);

gs_status_t read_cache_dispose(read_cache_t *cache);

gs_status_t read_cache_get(read_cache_t *cache, read_cache_counters_t *nullable_counters, bool *all_in_storage,
                           size_t *num_not_in_storage, bool **in_storage_mask, void **values, const void *keys,
                           size_t num_keys);

gs_status_t read_cache_lock(read_cache_t *cache);

gs_status_t read_cache_unlock(read_cache_t *cache);

gs_status_t read_cache_put(read_cache_t *cache, read_cache_counters_t *nullable_counters, const void *keys,
                           const void *values, size_t num_elements);

gs_status_t read_cache_get_key_size(size_t *key_size, const read_cache_t *cache);

gs_status_t read_cache_get_value_size(size_t *value_size, const read_cache_t *cache);

gs_status_t read_cache_get_capcity(size_t *capacity, const read_cache_t *cache);

gs_status_t read_cache_get_policy(replacement_policy_e *policy, const read_cache_t *cache);


#endif //GECKO_READ_CACHE_H
