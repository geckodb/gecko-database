#include <gecko-commons/gs_error.h>
#include <gecko-commons/gs_timer.h>

#ifndef GECKO_BUFFER_H
#define GECKO_BUFFER_H

typedef struct buffer_t buffer_t;

typedef struct slot_t {
    unsigned                  slot_id;
    const void                *key;
    const void                *value;
    struct {
        short                 in_use   : 1;
        short                 is_dirty : 1;
    } flags;
    size_t                    num_reads_since_fetch;
    size_t                    num_writes_since_fetch;
    size_t                    num_reads_total;
    size_t                    num_writes_total;
    timestamp_t               last_read;
    timestamp_t               last_mod;
} slot_t;

typedef struct buffer_counters_t
{
    unsigned read_miss;
    unsigned read_hit;
    unsigned write_miss;
    unsigned write_hit;

    unsigned num_evictions;
    unsigned num_loads;
} buffer_counters_t;

typedef enum buffer_replacement_e
{
    /* last recently used */
    buffer_replacement_lru
} buffer_replacement_e;

typedef enum buffer_write_policy_e
{
    /* write to both the buffer slot and the backing store */
    buffer_write_through,

    /* write only to the buffer slot and perform write to backing store later */
    buffer_write_back
} buffer_write_policy_e;

typedef enum buffer_alloc_policy_e
{
    /* A write operation always forces to also store the written value in a slot inside the buffer */
    buffer_alloc_fetch,

    /* A write operation always stores the written value in a slot inside the buffer if the old value was
     * already present in the buffer. Otherwise, only the backing store is updated but no slot is modified to
     * contain the value */
    buffer_alloc_write_around
} buffer_alloc_policy_e;

typedef gs_status_t (*buffer_fetch_func_t)(void **values, const void *keys, size_t num_elements);

typedef gs_status_t (*buffer_write_func_t)(const void *keys, const void *values, size_t num_elements);

/* compares 'needle' with all 'nhaystack' elements in 'haystack' and stores these 'nhaystack' comparison results
 * in 'result' */
typedef gs_status_t (*buffer_key_bulk_comp)(int *result, const void *needle, const slot_t *haystack, size_t nhaystack);

gs_status_t buffer_create(buffer_t **buf, size_t key_size, buffer_key_bulk_comp key_comp,
                          size_t value_size, size_t capacity,
                          buffer_fetch_func_t fetch_func, buffer_write_func_t write_func,
                          buffer_replacement_e repl_type, buffer_write_policy_e write_policy,
                          buffer_alloc_policy_e alloc_policy);

gs_status_t buffer_flush_all(buffer_t *buf);

gs_status_t buffer_dispose(buffer_t *buf);

gs_status_t buffer_get(buffer_counters_t *nullable_counters, void **values, const void *key, size_t num_keys);

gs_status_t buffer_lock(buffer_t *buf);

gs_status_t buffer_unlock(buffer_t *buf);

gs_status_t buffer_put(buffer_t *buf, buffer_counters_t *nullable_counters, const void *keys, const void *values, size_t num_elements);

gs_status_t buffer_get_key_size(size_t *key_size, const buffer_t *buf);

gs_status_t buffer_get_value_size(size_t *value_size, const buffer_t *buf);

gs_status_t buffer_get_capcity(size_t *capacity, const buffer_t *buf);

gs_status_t buffer_get_repl_type(buffer_replacement_e *repl_type, const buffer_t *buf);

gs_status_t buffer_get_write_policy(buffer_write_policy_e *write_policy, const buffer_t *buf);

gs_status_t buffer_get_alloc_policy(buffer_alloc_policy_e *alloc_policy, const buffer_t *buf);


#endif //GECKO_BUFFER_H
