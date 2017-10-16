#include <gs_system.h>
#include <gs_gridstore.h>
#include <gs_dispatcher.h>
#include <gs_shell.h>
#include <inet/gs_server.h>
#include <routers/api/1.0/root.h>
#include <routers/api/1.0/nodes.h>
#include <routers/catch.h>
#include <routers/api/1.0/collections.h>

typedef struct gs_system_t
{
    gs_gridstore_t    *gridstore;
    gs_dispatcher_t   *dispatcher;
    gs_shell_t        *shell;
    gs_server_pool_t  *server_pool;
} gs_system_t;

void setup_core(gs_system_t *system);
void setup_event_loop(gs_system_t *system);
void setup_shell(gs_system_t *system);
void setup_server(gs_system_t *system, unsigned short gateway_port);
void setup_events(gs_system_t *system);
void stop_system(gs_system_t *system);

gs_status_t gs_system_create(gs_system_t **system, unsigned short gateway_port)
{
    GS_REQUIRE_NONNULL(system);
    gs_system_t *result = GS_REQUIRE_MALLOC(sizeof(gs_system_t));

    setup_core(result);
    setup_event_loop(result);
    setup_shell(result);
    setup_server(result, gateway_port);
    setup_events(result);

    *system = result;

    return GS_SUCCESS;
}

gs_status_t gs_system_start(gs_system_t *system)
{
    GS_REQUIRE_NONNULL(system);

    gs_shell_start(system->shell, system);
    gs_dispatcher_start(system->dispatcher);

    return GS_SUCCESS;
}

gs_status_t gs_system_cleanup(gs_system_t *system)
{
    GS_REQUIRE_NONNULL(system);

    GS_DEBUG("dispose server pool %p", system->server_pool);
    while (gs_server_pool_dispose(system->server_pool) != GS_SUCCESS)
        ;

    // GS_DEBUG("dispose gridstore %p", gridstore);
    // while (gs_gridstore_dispose(gridstore) != GS_SUCCESS);

    GS_DEBUG("dispose shell %p", system->shell);
    while (gs_shell_dispose(&system->shell) != GS_SUCCESS)
        ;

    GS_DEBUG("dispose dispatcher %p", system->dispatcher);
    while (gs_dispatcher_dispose(&system->dispatcher) != GS_SUCCESS)
        ;

    apr_terminate();
    free (system);
    return GS_SUCCESS;
}

gs_gridstore_t *gs_system_get_gridstore(const gs_system_t *system)
{
    return (system != NULL ? system->gridstore : NULL);
}

gs_dispatcher_t *gs_system_get_dispatcher(const gs_system_t *system)
{
    return (system != NULL ? system->dispatcher : NULL);
}

gs_shell_t *gs_system_get_shell(const gs_system_t *system)
{
    return (system != NULL ? system->shell : NULL);
}

gs_server_pool_t *gs_system_get_server_pool(const gs_system_t *system)
{
    return (system != NULL ? system->server_pool : NULL);
}

void setup_core(gs_system_t *system)
{
    error_if((apr_initialize() != APR_SUCCESS), err_apr_initfailed);
    gs_gridstore_create(&system->gridstore);
}

void setup_event_loop(gs_system_t *system)
{
    error_if((gs_dispatcher_create(&system->dispatcher) != GS_SUCCESS), err_init_failed);
}

void setup_shell(gs_system_t *system)
{
    gs_shell_create(&system->shell, system);
}

void setup_server(gs_system_t *system, unsigned short gateway_port)
{
    gs_server_pool_create(&system->server_pool, system->dispatcher,
                          gateway_port,
                          "/api/1.0/",
                          router_api_1_0_root,
                          NUM_SERVERS);

    gs_server_pool_router_add(system->server_pool, "/api/1.0/nodes", router_api_1_0_nodes);
    gs_server_pool_router_add(system->server_pool, "/api/1.0/collections", router_api_1_0_collections);

    gs_server_pool_start(system->server_pool, system, router_catch);


}

void stop_system(gs_system_t *system)
{
    GS_DEBUG2("stopping system modules");
    gs_dispatcher_publish(system->dispatcher, gs_event_shell_shutdown(system, system->dispatcher, system->shell));
    gs_dispatcher_publish(system->dispatcher, gs_event_server_pool_shutdown(system, system->dispatcher, system->server_pool));
    gs_dispatcher_publish(system->dispatcher, gs_event_gridstore_shutdown(system, system->dispatcher, system->gridstore));
    gs_dispatcher_publish(system->dispatcher, gs_event_dispatcher_shutdown(system, system->dispatcher));
}

gs_status_t system_handle_events(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_SYSTEM);

    gs_signal_type_e  signal = GS_EVENT_GET_SIGNAL();
    gs_system_t      *system;

    gs_event_get_system(&system, event);

    switch (signal) {
        case GS_SIG_SYSEXIT:
        GS_DEBUG("system is exiting due to event %p", event);
            stop_system(system);
            return GS_CATCHED;
        default:
        warn("system %p received event for signal %d that is not handled", system->shell, signal);
            return GS_SKIPPED;
    }
}

void setup_events(gs_system_t *system)
{
    gs_status_t dispatcher_shutdown(const gs_event_t *event);

    GS_CONNECT(GS_SIG_SYSEXIT,  system_handle_events);


    GS_CONNECT(GS_SIG_SHUTDOWN, dispatcher_shutdown);

    GS_CONNECT(GS_SIG_SHUTDOWN, gs_shell_handle_events);

    GS_CONNECT(GS_SIG_SHUTDOWN, gs_gridstore_handle_events);

    GS_CONNECT(GS_SIG_SHUTDOWN, gs_server_pool_handle_events);

    GS_CONNECT(GS_SIG_TEST,     gs_gridstore_handle_events);
    GS_CONNECT(GS_SIG_INVOKE,   gs_gridstore_handle_events);
}

gs_status_t dispatcher_shutdown(const gs_event_t *event) {
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_DISPATCHER);

    gs_system_t *system;
    gs_event_get_system(&system, event);

    gs_dispatcher_shutdown(system->dispatcher, system);
    return GS_SUCCESS;
}