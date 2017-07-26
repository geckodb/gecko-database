#include <stdinc.h>

static char response[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html: charset=UTF-8\r\n\r\n"
        "<!doctype html>\r\n"
        "<html><head>MyTitle</head><body><h1>Hello World</h1></body></html>\r\n";

typedef struct server_t
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int server_desc;
    socklen_t socket_len;
} server_t;


void gs_server_create(server_t *server, in_port_t port);

void gs_server_start(server_t *server);