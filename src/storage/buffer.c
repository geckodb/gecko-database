#include <storage/buffer.h>
#include <gecko-commons/gecko-commons.h>

#define BUFFER_GET_PROPERTY(buffer, propname)     \
{                                                 \
    if (GS_LIKELY(buffer)) {                      \
        *propname = buffer->propname;             \
        return GS_SUCCESS;                        \
    } else return GS_FAILED;                      \
}

typedef struct lru_slot_t lru_slot_t;

typedef struct buffer_t buffer_t;

typedef struct buffer_t {
    size_t                      key_size;
    size_t                      value_size;
    size_t                      capacity;
    slot_t                     *slots;
    buffer_fetch_func_t         fetch_func;
    buffer_write_func_t         write_func;
    buffer_replacement_e        repl_type;
    buffer_write_policy_e       write_policy;
    buffer_alloc_policy_e       alloc_policy;
    void                       *buffer_impl;
    buffer_key_bulk_comp        key_comp;

    slot_t                     *(*get_victim)(buffer_t *self);
} buffer_t;

typedef struct lru_slot_t
{
    slot_t                    *slot;
    lru_slot_t                *prev, *next;
} lru_slot_t;

typedef struct buffer_impl_lru_t
{
    lru_slot_t                *first;
    lru_slot_t                *last;
} buffer_impl_lru_t;

slot_t *buffer_impl_lru_get_victim(buffer_t *self)
{
    buffer_impl_lru_t *lru_buffer = (buffer_impl_lru_t*) self->buffer_impl;

    /* The the last (oldest) slot and put it to the front (newest one) assuming the caller of the victim function
     * will modify the slots content */
    lru_slot_t *victim = lru_buffer->last;
    victim->slot->last_mod = gs_timer_now();

    /* The case where the buffer contains only one slot is not implemented */
    assert (lru_buffer->first != lru_buffer->last);

    lru_buffer->last        = lru_buffer->last->prev;
    lru_buffer->last->next  = NULL;
    victim->prev            = NULL;
    victim->next            = lru_buffer->first;
    lru_buffer->first->prev = victim;
    lru_buffer->first       = victim;

    return victim->slot;
}

gs_status_t buffer_create(buffer_t **buf, size_t key_size, buffer_key_bulk_comp key_comp,
                          size_t value_size, size_t capacity,
                          buffer_fetch_func_t fetch_func, buffer_write_func_t write_func,
                          buffer_replacement_e repl_type, buffer_write_policy_e write_policy,
                          buffer_alloc_policy_e alloc_policy)
{
    buffer_t *result = GS_REQUIRE_MALLOC(sizeof(buffer_t));
    result->key_size = key_size;
    result->value_size = value_size;
    result->capacity = capacity;
    result->fetch_func = fetch_func;
    result->write_func = write_func;
    result->repl_type = repl_type;
    result->write_policy = write_policy;
    result->alloc_policy = alloc_policy;
    result->key_comp = key_comp;
    result->slots = GS_REQUIRE_MALLOC(capacity * sizeof(slot_t));

    for (unsigned slot_id = 0; slot_id < capacity; slot_id++)
    {
        slot_t *slot = result->slots + slot_id;

        *slot = (slot_t) {
                .slot_id                    = slot_id,
                .flags.in_use               = FALSE,
                .last_mod                   = gs_timer_now(),
                .last_read                  = gs_timer_now(),
                .num_reads_since_fetch      = 0,
                .num_writes_since_fetch     = 0,
                .num_reads_total            = 0,
                .num_writes_total           = 0
        };
    }

    switch (repl_type) {
        case buffer_replacement_lru: {
            result->buffer_impl = GS_REQUIRE_MALLOC(sizeof(buffer_impl_lru_t));
            result->get_victim  = buffer_impl_lru_get_victim;

            buffer_impl_lru_t *lru_cache = (buffer_impl_lru_t *) result->buffer_impl;
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

    *buf = result;

    return GS_SUCCESS;
}

gs_status_t buffer_flush_all(buffer_t *buf)
{
    return GS_FAILED;
}

gs_status_t buffer_dispose(buffer_t *buf)
{
    return GS_FAILED;
}

gs_status_t buffer_get(buffer_counters_t *nullable_counters, void **values, const void *key, size_t num_keys)
{

    return GS_FAILED;
}

gs_status_t buffer_lock(buffer_t *buf)
{
    return GS_FAILED;
}

gs_status_t buffer_unlock(buffer_t *buf)
{
    return GS_FAILED;
}

gs_status_t buffer_put(buffer_t *buf, buffer_counters_t *nullable_counters, const void *keys, const void *values,
                       size_t num_elements)
{
    slot_t        *free_slot;

    int           *comp_result = GS_REQUIRE_MALLOC(buf->capacity * sizeof(int));

    while (num_elements--)
    {
        const void *key = keys;
        const void *value = values;

        // Compare the key with all keys currently stored in the slots
        // TODO: Check whether data parallelism makes sense here
        if (buf->key_comp(comp_result, key, buf->slots, buf->capacity) != GS_SUCCESS) {
            free (comp_result);
            return GS_FAILED;
        }

        // Find the slot which contains the key (if any)
        slot_t *found_slot = NULL;
        for (unsigned slot_id = 0; slot_id < buf->capacity; slot_id++) {
            if (GS_UNLIKELY(comp_result[slot_id] == 0)) {
                found_slot = buf->slots + slot_id;
                break;
            }
        }

        if (GS_LIKELY(found_slot)) {
            /* There is a slot in the buffer that holds the old value */

            /* Perform a value update on this slot */
            found_slot->value = value;

            /* Depending on the write policy, update the backing store directly or later */
            switch (buf->write_policy) {
                case buffer_write_through:
                    buf->write_func(key, value, 1);
                    found_slot->flags.is_dirty = FALSE;
                    break;
                case buffer_write_back:
                    found_slot->flags.is_dirty = TRUE;
                    break;
                default:
                    free (comp_result);
                    return GS_INTERNALERR;
            }
        } else {
            /* There is no slot in the buffer that holds the old value */

            /* Depending on the allocation policy, just update the backing store directly and additionally store
             * the value into the buffer */
            switch (buf->alloc_policy) {
                case buffer_alloc_fetch:
                    free_slot = buf->get_victim(buf);
                    if (free_slot->flags.is_dirty) {
                        buf->write_func(free_slot->key, free_slot->value, 1);
                        free_slot->flags.is_dirty = FALSE;
                        free_slot->num_writes_since_fetch = 1;
                        free_slot->num_writes_total++;
                    }
                    buf->write_func(key, value, 1);
                    free_slot->flags.in_use = TRUE;
                    free_slot->key = key;
                    free_slot->value = value;
                    break;
                case buffer_alloc_write_around:
                    buf->write_func(key, value, 1);
                    break;
                default:
                    free (comp_result);
                    return GS_INTERNALERR;
            }
        }

        keys += buf->key_size;
        values += buf->value_size;
    }

    free (comp_result);
    return GS_FAILED;
}

gs_status_t buffer_get_key_size(size_t *key_size, const buffer_t *buf)
{
    BUFFER_GET_PROPERTY(buf, key_size)
}

gs_status_t buffer_get_value_size(size_t *value_size, const buffer_t *buf)
{
    BUFFER_GET_PROPERTY(buf, value_size)
}

gs_status_t buffer_get_capcity(size_t *capacity, const buffer_t *buf)
{
    BUFFER_GET_PROPERTY(buf, capacity)
}

gs_status_t buffer_get_repl_type(buffer_replacement_e *repl_type, const buffer_t *buf)
{
    BUFFER_GET_PROPERTY(buf, repl_type)
}

gs_status_t buffer_get_write_policy(buffer_write_policy_e *write_policy, const buffer_t *buf)
{
    BUFFER_GET_PROPERTY(buf, write_policy)
}

gs_status_t buffer_get_alloc_policy(buffer_alloc_policy_e *alloc_policy, const buffer_t *buf)
{
    BUFFER_GET_PROPERTY(buf, alloc_policy)
}