#include <gs_dispatcher.h>
#include <containers/dicts/hash_table.h>
#include <gs_spinlock.h>

typedef struct gs_dispatcher_t
{
    bool            accept_new;
    apr_queue_t    *queue;
    apr_pool_t     *mem_pool;
    dict_t         *handler_map;
} gs_dispatcher_t;

 vec_t *dispatcher_get_handler(gs_dispatcher_t *dispatcher, gs_signal_type_e signal);

GS_DECLARE(gs_status_t) gs_dispatcher_create(gs_dispatcher_t **dispatcher)
{
    gs_dispatcher_t *result = GS_REQUIRE_MALLOC(sizeof(gs_dispatcher_t));
    result->accept_new = true;
    apr_pool_create(&result->mem_pool, NULL);
    apr_queue_create(&result->queue, MESSAGE_QUEUE_SIZE_MAX, result->mem_pool);
    result->handler_map = hash_table_new_defaults(sizeof(gs_signal_type_e), sizeof(vec_t));
    *dispatcher = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_dispatcher_dispose(gs_dispatcher_t **dispatcher_ptr)
{
    GS_REQUIRE_NONNULL(dispatcher_ptr)
    GS_REQUIRE_NONNULL(*dispatcher_ptr)
    gs_dispatcher_t *dispatcher = *dispatcher_ptr;
    apr_pool_destroy(dispatcher->mem_pool);
    dict_clear(dispatcher->handler_map);
    GS_DEBUG("dispatcher %p disposed", dispatcher);
    free(dispatcher);
    *dispatcher_ptr = NULL;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_dispatcher_start(gs_dispatcher_t *dispatcher)
{
    GS_REQUIRE_NONNULL(dispatcher)

    apr_status_t status;
    gs_signal_type_e signal;
    gs_event_t *event;
    vec_t *handlers;

    GS_DEBUG("dispatcher %p is starting", dispatcher);

    while (true) {
        GS_DEBUG("dispatcher %p is waiting for events", dispatcher);
        if (!dispatcher->accept_new && (apr_queue_size(dispatcher->queue) == 0)) {
            break;
        }
        if ((status = apr_queue_pop(dispatcher->queue, (void **) &event)) != APR_EINTR) {
            error_if((status == APR_EOF), err_dispatcher_terminated);
            assert (status == APR_SUCCESS && event != NULL);
            signal = gs_event_get_signal(event);
            handlers = dispatcher_get_handler(dispatcher, signal);
            WARN_IF(((signal != GS_SIG_HEARTBEAT) && (!handlers)), "no handler for signal '%d' registered", signal);
            GS_DEBUG("dispatcher %p received event for signal %d", dispatcher, signal);
            for (gs_event_handler_t *it = vec_begin(handlers);
                (it && (it != vec_end(handlers))); it++) {
                (*it)(event); // TODO: handler can return void
            }
            gs_event_free(event);
        }
    }
    GS_DEBUG("dispatcher %p left main loop", dispatcher);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_dispatcher_shutdown(gs_dispatcher_t *dispatcher)
{
    GS_REQUIRE_NONNULL(dispatcher)
    GS_DEBUG("initialize shutdown for dispatcher %p", dispatcher);
    gs_dispatcher_publish(dispatcher, gs_event_heartbeat_new(dispatcher));
    dispatcher->accept_new = false;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_dispatcher_connect(gs_dispatcher_t *dispatcher, gs_signal_type_e signal, gs_event_handler_t handler)
{
    GS_REQUIRE_NONNULL(dispatcher)
    GS_REQUIRE_NONNULL(handler)
    if (!dict_contains_key(dispatcher->handler_map, &signal)) {
        vec_t *vec = vec_new(sizeof(gs_event_handler_t), 10);
        dict_put(dispatcher->handler_map, &signal, vec);
        free (vec);
    }
    vec_t *handlers = (vec_t *) dict_get(dispatcher->handler_map, &signal);
    vec_pushback(handlers, 1, &handler);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_dispatcher_publish(gs_dispatcher_t *dispatcher, gs_event_t *event)
{
    GS_REQUIRE_NONNULL(dispatcher)
    GS_REQUIRE_NONNULL(event)
    if (dispatcher->accept_new) {
        apr_status_t status;
        while ((status = apr_queue_push(dispatcher->queue, event)) == APR_EINTR);
        error_if((status != APR_SUCCESS), err_internal);
        return GS_SUCCESS;
    } else {
        GS_DEBUG("dispatcher %p is shutting down and skipped event %p", dispatcher, event);
        return GS_SKIPPED;
    }
}

GS_DECLARE(gs_status_t) gs_dispatcher_waitfor(gs_dispatcher_t *dispatcher, gs_event_t *event)
{
    volatile gs_spinlock_t *lock;
    gs_spinlock_create(&lock);
    gs_dispatcher_publish(dispatcher, gs_event_new_blocking(lock, event));
    GS_DEBUG2("thread is being locked...");
    gs_spinlock_lock(lock);
    GS_DEBUG2("thread was unlocked...");
    gs_spinlock_dispose(&lock);
    return GS_SUCCESS;
}

 vec_t *dispatcher_get_handler(gs_dispatcher_t *dispatcher, gs_signal_type_e signal)
{
    return (vec_t *) dict_get(dispatcher->handler_map, &signal);
}