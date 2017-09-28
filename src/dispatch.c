#include <dispatch.h>
#include <containers/dicts/hash_table.h>

void dispatch_create(dispatcher_t *dispatcher)
{
    REQUIRE_NONNULL(dispatcher)
    dispatcher->is_running = false;
    apr_pool_create(&dispatcher->mem_pool, NULL);
    apr_queue_create(&dispatcher->queue, MESSAGE_QUEUE_SIZE_MAX, dispatcher->mem_pool);
    dispatcher->handler_map = hash_table_new_defaults(sizeof(message_type), sizeof(vec_t));
}

void dispatch_dispose(dispatcher_t *dispatcher)
{
    REQUIRE_NONNULL(dispatcher)
}

int dispatch_run(dispatcher_t *dispatcher)
{
    REQUIRE_NONNULL(dispatcher)

    const event_t *event = NULL;
    apr_status_t status;

    dispatcher->is_running = true;

    while (dispatcher->is_running) {
        while ((status = apr_queue_pop(dispatcher->queue, (void **) &event)) == APR_EINTR) {
            error_if((status == APR_EOF), err_dispatcher_terminated);
            assert (status == APR_SUCCESS && event != NULL);

        }

    }
    return 0;
}

hander_id_t dispatch_connect(dispatcher_t *dispatcher, message_type message, message_handler_t handler)
{
    REQUIRE_NONNULL(dispatcher)
    REQUIRE_NONNULL(handler)
    return 0;
}

int dispatch_disconnect(dispatcher_t *dispatcher, hander_id_t handler_id)
{
    REQUIRE_NONNULL(dispatcher)
    return 0;
}