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

#include <stdinc.h>
#include <argp.h>
#include <info.h>
#include <dispatcher.h>

// curl -i -G -d "key=val" -d "abs=[1,2,3,4]" http://localhost:36895/api/test

gs_gridstore_t    *gridstore;
gs_dispatcher_t   *dispatcher;

static inline void setup_config(int argc, char **argv);
static inline void setup_core();
static inline void setup_shell();
static inline void setup_server();
static inline void setup_events();
static inline void start_system();

int main(int argc, char* argv[])
{
    setup_config(argc, argv);
    setup_core();
    setup_shell();
    setup_server();
    setup_events();

    gs_dispatcher_publish(dispatcher, gs_event_gridstore_test(gridstore));
    gs_dispatcher_publish(dispatcher, gs_event_dispatcher_shutdown(dispatcher));

    start_system();



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

static inline void setup_shell()
{

}

static inline void setup_server()
{
    /*server_t server;
  server_create(&server, port_num, NULL);
  server_router_add(&server, "/api/types/create", router_api_types_create);
  server_start(&server, router_catch);*/
}

static inline void setup_events()
{
    gs_status_t dispatcher_shutdown(const gs_event_t *event);

    error_if((gs_dispatcher_create(&dispatcher) != GS_SUCCESS), err_init_failed);

    GS_CONNECT(GS_SIG_SHUTDOWN, dispatcher_shutdown);

    GS_CONNECT(GS_SIG_TEST,     gs_gridstore_handle_events);
}

static inline void start_system()
{
    gs_dispatcher_start(dispatcher);
}

gs_status_t dispatcher_shutdown(const gs_event_t *event) {
    gs_dispatcher_shutdown(dispatcher);
    return GS_SUCCESS;
}