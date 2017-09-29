// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs.h>
#include <argp.h>
#include <info.h>
#include <gs_dispatcher.h>
#include <gs_shell.h>
#include <routers/api/types/create/router.h>
#include <routers/catch.h>
#include <inet/gs_server.h>

// curl -i -G -d "key=val" -d "abs=[1,2,3,4]" http://localhost:36895/api/test

gs_gridstore_t    *gridstore;
gs_dispatcher_t   *dispatcher;
gs_shell_t        *shell;
gs_server_t       *server;

static inline void setup_config(int argc, char **argv);
static inline void setup_core();
static inline void setup_event_loop();
static inline void setup_shell();
static inline void setup_server();
static inline void setup_events();
static inline void start_system();
static inline void stop_system();
static inline void cleanup();
static inline void stop_event_loop();

int main(int argc, char* argv[])
{
    setup_config(argc, argv);
    setup_core();
    setup_event_loop();
    setup_shell();
    setup_server();
    setup_events();

    start_system();

    cleanup();
    stop_event_loop();

    //gs_dispatcher_shutdown(dispatcher);

    apr_terminate();

    return EXIT_SUCCESS;
}

static inline void setup_config(int argc, char **argv)
{
    startup_config.port = 35497;

    argp_parse (&argp, argc, argv, 0, 0, &startup_config);
}

static inline void setup_core()
{
    error_if((apr_initialize() != APR_SUCCESS), err_apr_initfailed);
    gs_gridstore_create(&gridstore);
}

static inline void setup_event_loop()
{
    error_if((gs_dispatcher_create(&dispatcher) != GS_SUCCESS), err_init_failed);
}

static inline void setup_shell()
{
    gs_shell_create(&shell, dispatcher);
}

static inline void setup_server()
{
    gs_server_create(&server, startup_config.port, NULL, dispatcher);
    gs_server_router_add(server, "/api/types/create", router_api_types_create);
    gs_server_start(server, router_catch);

    /*gs_server_t server;
  gs_server_create(&server, port_num, NULL);
  gs_server_router_add(&server, "/api/types/create", router_api_types_create);
  gs_server_start(&server, router_catch);*/
}

static inline void stop_system()
{
    GS_DEBUG2("stopping system modules");
    gs_dispatcher_publish(dispatcher, gs_event_shell_shutdown(dispatcher, shell));
    gs_dispatcher_publish(dispatcher, gs_event_server_shutdown(dispatcher, server));
    gs_dispatcher_publish(dispatcher, gs_event_gridstore_shutdown(dispatcher, gridstore));
    gs_dispatcher_publish(dispatcher, gs_event_dispatcher_shutdown(dispatcher));

}

static inline void cleanup()
{
    //GS_DEBUG("dispose server %p", server);
   // while (gs_server_dispose(&server) != GS_SUCCESS)
   //     ;

   // GS_DEBUG("dispose gridstore %p", gridstore);
   // while (gs_gridstore_dispose(gridstore) != GS_SUCCESS);

    GS_DEBUG("dispose shell %p", shell);
    while (gs_shell_dispose(&shell) != GS_SUCCESS)
        ;

    GS_DEBUG("dispose dispatcher %p", dispatcher);
    while (gs_dispatcher_dispose(&dispatcher) != GS_SUCCESS)
        ;
}

static inline void stop_event_loop()
{
    //GS_DEBUG("shutting down dispatcher %p", dispatcher);
    //gs_dispatcher_shutdown(dispatcher);
}

static inline gs_status_t system_handle_events(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_SYSTEM);

    gs_signal_type_e signal = GS_EVENT_GET_SIGNAL();

    switch (signal) {
        case GS_SIG_SYSEXIT:
            GS_DEBUG("system is exiting due to event %p", event);
            stop_system();
            return GS_CATCHED;
        default:
        warn("system %p received event for signal %d that is not handled", shell, signal);
            return GS_SKIPPED;
    }
}

static inline void setup_events()
{
    gs_status_t dispatcher_shutdown(const gs_event_t *event);

    GS_CONNECT(GS_SIG_SYSEXIT,  system_handle_events);


    GS_CONNECT(GS_SIG_SHUTDOWN, dispatcher_shutdown);

    GS_CONNECT(GS_SIG_SHUTDOWN, gs_shell_handle_events);

    GS_CONNECT(GS_SIG_SHUTDOWN, gs_gridstore_handle_events);

    GS_CONNECT(GS_SIG_SHUTDOWN, gs_server_handle_events);

    GS_CONNECT(GS_SIG_TEST,     gs_gridstore_handle_events);
}

static inline void start_system()
{
    gs_shell_start(shell);
    gs_dispatcher_start(dispatcher);
}

gs_status_t dispatcher_shutdown(const gs_event_t *event) {
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_DISPATCHER);

    gs_dispatcher_shutdown(dispatcher);
    return GS_SUCCESS;
}