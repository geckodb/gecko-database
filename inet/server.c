#include <inet/server.h>
#include <conf.h>
#include <debug.h>
#include <inet/response.h>

void gs_server_create(server_t *server, in_port_t port)
{
    REQUIRE_NONNULL(server);

    server->socket_len = sizeof(server->client_addr);
    int on = 1;

    server->server_desc = socket(AF_INET, SOCK_STREAM, 0);
    panic_if((server->server_desc < 0), "server socket creation failed (%s).", strerror(errno));

    setsockopt(server->server_desc, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = port;

    int succ = bind(server->server_desc, (const struct sockaddr *) &server->server_addr, sizeof(server->server_addr));
    panic_if((succ < 0), "unable to bind to port %d (%s).", (int) ntohs(server->server_addr.sin_port), strerror(errno));

    if (listen(server->server_desc, 10) < 0) {
        close(server->server_desc);
        panic("unable to start listening to port %d (%s).", (int) ntohs(server->server_addr.sin_port), strerror(errno));
    }

    printf("listening to port %d\n", (int) ntohs(server->server_addr.sin_port));
}

void gs_server_start(server_t *server)
{
    REQUIRE_NONNULL(server);
    int fd_client;
    char buffer[2048];

    while (1) {
        if ((fd_client = accept(server->server_desc, (struct sockaddr*) &server->client_addr, &server->socket_len)) < 0) {
//            LOG_WARN("unable to accept incoming connection (%s)", strerror(errno));
            close (fd_client);
            continue;
        } else {
            //if (!fork()) {
             //   LOG_DEBUG("established connection, client-adr:%s:%d",
            //              inet_ntoa(server->client_addr.sin_addr),
            //             (int) ntohs(server->client_addr.sin_port));
                close (server->server_desc);
                memset (buffer, 0, 2048);
                read(fd_client, buffer, 2047);
          //      LOG_DEBUG("incoming request\n--------------------------------------------------------------\n%s", buffer);

                response_t res;
                gs_response_create(&res);
                gs_response_content_type_set(&res, MIME_CONTENT_TYPE_TEXT_PLAIN);
                gs_response_body_set(&res, "Hello World");
                gs_response_end(&res, HTTP_STATUS_CODE_200_OK);
                char *response_text = gs_response_pack(&res);

                write(fd_client, response_text, strlen(response_text));
                free (response_text);
                gs_response_free(&res);


                close (fd_client);
            //    LOG_DEBUG("connection closed, client-adr:%s:%d", inet_ntoa(server->client_addr.sin_addr), (int) ntohs(server->client_addr.sin_port));
                exit(0);
            //}
        }
        close (fd_client);
       // LOG_DEBUG("connection closed, client-adr:%s:%d", inet_ntoa(server->client_addr.sin_addr), (int) ntohs(server->client_addr.sin_port));
    }
}