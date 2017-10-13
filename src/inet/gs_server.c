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

#include <inet/gs_server.h>
#include <debug.h>
#include <inet/response.h>
#include <sys/socket.h>
#include <poll.h>
#include <apr_pools.h>
#include <c11threads.h>
#include <stdatomic.h>
#include <inet/gs_request.h>
#include <apr_hash.h>
#include <apr_strings.h>
#include <containers/gs_hash.h>

typedef struct gs_server_t
{
    gs_dispatcher_t     *dispatcher;
    apr_pool_t          *pool;
    struct sockaddr_in   server_addr;
    struct sockaddr_in   client_addr;
    int                  server_desc;
    socklen_t            socket_len;
    gs_hash_t           *routers;
    thrd_t               thread;
    bool                 is_running;
    bool                 is_disposable;
} gs_server_t;

typedef struct gs_server_pool_t
{
    gs_server_t         *gateway;
    vec_t               *servers;
} gs_server_pool_t;

typedef struct server_loop_args_t
{
    gs_server_t         *server;
    router_t             catch;
} server_loop_args_t;

typedef struct server_handle_connection_args_t
{
    int client;
    server_loop_args_t *loop_args;

} server_handle_connection_args_t;

void
str_clean_up(
        void *key,
        void *value)
{
    char **key_string = (char **) key;
    free (*key_string);
}

int server_loop(void *args);
int server_handle_connection(void *args);

GS_DECLARE(gs_status_t) gs_server_create(gs_server_t **out, unsigned short port, gs_dispatcher_t *dispatcher)
{
    gs_server_t *server = GS_REQUIRE_MALLOC(sizeof(gs_server_t));

    server->socket_len = sizeof(server->client_addr);
    server->dispatcher = dispatcher;
    server->is_disposable = false;
    server->is_running = false;
    apr_pool_create(&server->pool, NULL);

    int on = 1;

    server->server_desc = socket(AF_INET, SOCK_STREAM, 0);
    panic_if((server->server_desc < 0), "gateway socket creation failed (%s).", strerror(errno));

    struct timeval timeout = {
        .tv_sec = 10
    };

    setsockopt(server->server_desc, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    setsockopt(server->server_desc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(port);

    int succ = bind(server->server_desc, (const struct sockaddr *) &server->server_addr, sizeof(server->server_addr));
    panic_if((succ < 0), "unable to bind to port %d (%s).", (int) ntohs(server->server_addr.sin_port), strerror(errno));

    if (listen(server->server_desc, 10) < 0) {
        close(server->server_desc);
        panic("unable to start_system listening to port %d (%s).", (int) ntohs(server->server_addr.sin_port), strerror(errno));
    }

    socklen_t len = sizeof(struct sockaddr_in);
    succ = (getsockname(server->server_desc, (struct sockaddr *) & server->server_addr, &len));


  /*  gateway->routers = hash_table_new_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                        sizeof(char *), sizeof(router_t), RESPONSE_DICT_CAPACITY,
                                        RESPONSE_DICT_CAPACITY, 1.7f, 0.75f,
                                        str_equals, str_clean_up, true);*/
    gs_hash_create(&server->routers, 10, GS_STRING_COMP);

    printf("listening to port %d\n", (int) ntohs(server->server_addr.sin_port));

    *out = server;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_server_router_add(gs_server_t *server, const char *resource, router_t router)
{
    GS_REQUIRE_NONNULL(server)
   // dict_put(gateway->routers, &resource, &router);
    char *key = apr_pstrdup(server->pool, resource);
    gs_hash_set(server->routers, key, sizeof(char *), router, GS_STRING_COMP);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_server_start(gs_server_t *server, router_t catch)
{
    GS_REQUIRE_NONNULL(server);
    GS_REQUIRE_NONNULL(catch);

    if (!server->is_running) {
        GS_DEBUG("gateway %p is starting", server);
        server->is_running = true;

        server_loop_args_t *args = GS_REQUIRE_MALLOC(sizeof(server_loop_args_t));
        args->server = server;
        args->catch = catch;

        thrd_create(&server->thread, server_loop, args);
        return GS_SUCCESS;
    } else {
        warn("gateway %p was request to start but runs already", server);
        return GS_FAILED;
    }
}

GS_DECLARE(unsigned short) gs_server_port(const gs_server_t *server)
{
    GS_REQUIRE_NONNULL(server);
    return ntohs(server->server_addr.sin_port);
}

GS_DECLARE(gs_status_t) gs_server_dispose(gs_server_t *server)
{
    GS_REQUIRE_NONNULL(server);
    if (server->is_disposable) {
        apr_pool_destroy(server->pool);
     //   dict_clear(gateway->routers);
        GS_DEBUG("gateway %p disposed", server);
        free(server);
        return GS_SUCCESS;
    } else {
        return GS_TRYAGAIN;
    }
}

GS_DECLARE(gs_status_t) gs_server_shutdown(gs_server_t *server)
{
    if (server->is_running) {
        GS_DEBUG("gateway %p is shutting down", server);
        server->is_running = false;
        return GS_SUCCESS;
    } else {
        GS_DEBUG("gateway %p is already shutting down", server);
        return GS_FAILED;
    }
}

int server_loop(void *args)
{
    GS_REQUIRE_NONNULL(args);
    server_loop_args_t *loop_args = (server_loop_args_t *) args;

    int client;
    fd_set set;

    while (loop_args->server->is_running) {
        FD_ZERO(&set);
        FD_SET(loop_args->server->server_desc, &set);

        struct timeval timeout = {
            .tv_sec = SERVER_SELECT_TIMEOUT
        };

        int select_result = select(loop_args->server->server_desc + 1, &set, NULL, NULL, &timeout);
        panic_if((select_result == -1), "unable to set timeout on %d", (loop_args->server->server_desc + 1));
        if(select_result == 0) {
            //GS_DEBUG2("timeout reached for accept incoming connection");
            continue;
        }

        if ((client = accept(loop_args->server->server_desc, (struct sockaddr*) &loop_args->server->client_addr,
                             &loop_args->server->socket_len)) < 0) {
            close (client);
            continue;
        } else {

            thrd_t thrd;
            server_handle_connection_args_t *args = GS_REQUIRE_MALLOC(sizeof(server_handle_connection_args_t));
            args->loop_args = loop_args;
            args->client = client;
            thrd_create(&thrd, server_handle_connection, args);


        }
        //close (client);
    }
    GS_DEBUG("gateway %p left main loop", loop_args->server);
    loop_args->server->is_disposable = true;

    return EXIT_SUCCESS;
}

int server_handle_connection(void *args)
{
    server_handle_connection_args_t *server_handle_connection_args = (server_handle_connection_args_t *) args;

    response_t response;
    char buffer[131072];
    int client = server_handle_connection_args->client;
    server_loop_args_t *loop_args = server_handle_connection_args->loop_args;

    memset (buffer, 0, sizeof(buffer));
    struct pollfd pollfd = { .fd = client, .events = POLLIN };
    response_create(&response);
    if (poll(&pollfd, 1, 100000)) {
        gs_request_t *request;
        gs_request_create(&request, client);

        if (!gs_request_is_valid(request)) {
            GS_DEBUG2("request rejected");
            response_content_type_set(&response, MIME_CONTENT_TYPE_TEXT_PLAIN);
            response_end(&response, HTTP_STATUS_CODE_400_BAD_REQUEST);
        } else {
            router_t router;
            char *resource;
            gs_request_resource(&resource, request);
            GS_DEBUG("loop args %p", loop_args);
            GS_DEBUG("resource %s", resource);
            GS_DEBUG("gateway %p", loop_args->server);
            GS_DEBUG("routers %p", loop_args->server->routers);
            if ((router = (router_t) gs_hash_get(loop_args->server->routers, resource, sizeof(char *))) != NULL) {
                GS_DEBUG("request delegated to router for resource '%s'", resource);
                router(loop_args->server->dispatcher, request, &response);
            } else {
                GS_DEBUG("no router installed for resource '%s'; delegate to default router", resource);
                loop_args->catch(loop_args->server->dispatcher, request, &response);
            }
        }

        gs_request_dispose(&request);
    } else {
        response_end(&response, HTTP_STATUS_CODE_408_REQUEST_TIMEOUT);
    }
    char *response_text = response_pack(&response);
    GS_DEBUG("RESPONSE TEXT: %s\n", response_text);
    write(client, response_text, strlen(response_text));
    free(response_text);
    response_dispose(&response);
    close(client);

    return EXIT_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_server_pool_create(gs_server_pool_t **server_set, gs_dispatcher_t *dispatcher,
                                              unsigned short gateway_port, const char *gateway_resource, router_t router,
                                              size_t num_servers)
{
    GS_REQUIRE_NONNULL(dispatcher);
    gs_server_pool_t *result = GS_REQUIRE_MALLOC(sizeof(gs_server_pool_t));

    result->servers = vec_new(sizeof(gs_server_t *), num_servers);
    vec_resize(result->servers, num_servers);
    for (gs_server_t **it = vec_begin(result->servers); it != vec_end(result->servers); it++) {
        gs_server_create(it, 0, dispatcher);
        GS_DEBUG("created server listening to port: %d", gs_server_port(*it));
    }

    gs_server_create(&result->gateway, gateway_port, dispatcher);
    gs_server_router_add(result->gateway, gateway_resource, router);

    *server_set = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_server_pool_dispose(gs_server_pool_t *pool)
{
    GS_REQUIRE_NONNULL(pool);

    GS_DEBUG("dispose gateway %p", pool->gateway);
    while (gs_server_dispose(pool->gateway) != GS_SUCCESS)
        ;

    for (gs_server_t **it = vec_begin(pool->servers); it != vec_end(pool->servers); it++) {
        GS_DEBUG("dispose server %p", *it);
        while (gs_server_dispose(*it) != GS_SUCCESS)
            ;
    }

    vec_free(pool->servers);
    free (pool);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_server_pool_router_add(gs_server_pool_t *pool, const char *resource, router_t router)
{
    GS_REQUIRE_NONNULL(pool);
    GS_REQUIRE_NONNULL(resource);
    GS_REQUIRE_NONNULL(router);

    gs_status_t status;

    for (gs_server_t **it = vec_begin(pool->servers); it != vec_end(pool->servers); it++) {
        GS_DEBUG("add router to server %p", *it);
        if ((status = gs_server_router_add(*it, resource, router)) != GS_SUCCESS)
            return status;
    }

    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_server_pool_start(gs_server_pool_t *pool, router_t catch)
{
    GS_REQUIRE_NONNULL(pool);
    GS_REQUIRE_NONNULL(catch);

    gs_status_t status;

    for (gs_server_t **it = vec_begin(pool->servers); it != vec_end(pool->servers); it++) {
        GS_DEBUG("start server %p", *it);
        if ((status = gs_server_start(*it, catch)) != GS_SUCCESS)
            return status;
    }

    GS_DEBUG("start gateway %p", pool->gateway);
    if ((status = gs_server_start(pool->gateway, catch)) != GS_SUCCESS)
        return status;

    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_server_pool_handle_events(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_SERVER_POOL);

    gs_server_pool_t *pool   = GS_EVENT_GET_RECEIVER(gs_server_pool_t);
    gs_signal_type_e  signal = GS_EVENT_GET_SIGNAL();

    switch (signal) {
        case GS_SIG_SHUTDOWN:
        GS_DEBUG("server pool %p received shutdown signal", pool);
            gs_server_pool_shutdown(pool);
            return GS_SKIPPED;
        default:
        warn("server pool %p received event for signal %d that is not handled", pool, signal);
            return GS_SKIPPED;
    }
}

GS_DECLARE(gs_status_t) gs_server_pool_shutdown(gs_server_pool_t *pool)
{
    GS_REQUIRE_NONNULL(pool);

    gs_status_t status;

    if ((status = gs_server_shutdown(pool->gateway)) != GS_SUCCESS)
        return status;

    for (gs_server_t **it = vec_begin(pool->servers); it != vec_end(pool->servers); it++) {
        GS_DEBUG("start server %p", *it);
        if ((status = gs_server_shutdown(*it)) != GS_SUCCESS)
            return status;
    }

    return GS_SUCCESS;
}