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

#include <inet/server.h>
#include <conf.h>
#include <debug.h>
#include <inet/response.h>
#include <inet/request.h>
#include <sys/socket.h>
#include <poll.h>
#include <containers/dicts/hash_table.h>

void
str_clean_up(
        void *key,
        void *value)
{
    char **key_string = (char **) key;
    free (*key_string);
}

void server_create(server_t *server, unsigned short port, capture_t *capture)
{
    GS_REQUIRE_NONNULL(server);

    server->socket_len = sizeof(server->client_addr);
    int on = 1;

    server->server_desc = socket(AF_INET, SOCK_STREAM, 0);
    panic_if((server->server_desc < 0), "server socket creation failed (%s).", strerror(errno));

    setsockopt(server->server_desc, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(port);
    server->capture = capture;

    int succ = bind(server->server_desc, (const struct sockaddr *) &server->server_addr, sizeof(server->server_addr));
    panic_if((succ < 0), "unable to bind to port %d (%s).", (int) ntohs(server->server_addr.sin_port), strerror(errno));

    if (listen(server->server_desc, 10) < 0) {
        close(server->server_desc);
        panic("unable to start_system listening to port %d (%s).", (int) ntohs(server->server_addr.sin_port), strerror(errno));
    }

    server->routers = hash_table_new_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                        sizeof(char *), sizeof(router_t), RESPONSE_DICT_CAPACITY,
                                        RESPONSE_DICT_CAPACITY, 1.7f, 0.75f,
                                        str_equals, str_clean_up, true);

    printf("listening to port %d\n", (int) ntohs(server->server_addr.sin_port));
}

void server_router_add(server_t *server, const char *resource, router_t router)
{
    GS_REQUIRE_NONNULL(server)
    dict_put(server->routers, &resource, &router);
}

void server_start(server_t *server, router_t catch)
{
    GS_REQUIRE_NONNULL(server);
    int client;
    char buffer[131072];
    request_t request;
    response_t response;

    while (true) {
        if ((client = accept(server->server_desc, (struct sockaddr*) &server->client_addr, &server->socket_len)) < 0) {
            close (client);
            continue;
        } else {
            memset (buffer, 0, sizeof(buffer));
            struct pollfd pollfd = { .fd = client, .events = POLLIN };
            response_create(&response);
            if (poll(&pollfd, 1, 1000)) {
                    recv(client, buffer, sizeof(buffer), 0);
                    request_parse(&request, buffer);
                    request_print(stdout, &request);
                    if (!request.valid) {
                        response_content_type_set(&response, MIME_CONTENT_TYPE_TEXT_PLAIN);
                        response_end(&response, HTTP_STATUS_CODE_400_BAD_REQUEST);
                    } else {
                        const router_t *router;
                        if ((router = dict_get(server->routers, &request.resource)) != NULL) {
                            (*router)(server->capture, &request, &response);
                        } else catch(server->capture, &request, &response);
                    }
            } else {
                response_end(&response, HTTP_STATUS_CODE_408_REQUEST_TIMEOUT);
            }
            char *response_text = response_pack(&response);
            write(client, response_text, strlen(response_text));
            free(response_text);
            response_dispose(&response);
            close(client);
        }
        close (client);
    }
}

GS_DECLARE(gs_status_t) gs_server_handle_events(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_SERVER);

    server_t         *server = GS_EVENT_GET_RECEIVER(server_t);
    gs_signal_type_e  signal = GS_EVENT_GET_SIGNAL();

    switch (signal) {
        case GS_SIG_SHUTDOWN:
            printf("server shutdown\n");
            return GS_SKIPPED;
        default:
        warn("server %p received event for signal %d that is not handled", server, signal);
            return GS_SKIPPED;
    }
}
