// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdinc.h>
#include "response.h"
#include "request.h"

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

static char response[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html: charset=UTF-8\r\n\r\n"
        "<!doctype html>\r\n"
        "<html><head>MyTitle</head><body><h1>Hello World</h1></body></html>\r\n";

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct server_t
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int server_desc;
    socklen_t socket_len;
    dict_t *routers;
} server_t;

typedef void (*router_t)(const request_t *req, response_t *res);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void server_create(server_t *server, in_port_t port);
void server_router_add(server_t *server, const char *resource, router_t router);
void server_start(server_t *server, router_t catch);