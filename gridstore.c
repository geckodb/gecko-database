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

#include <argp.h>

#include <stdio.h>
#include <grid.h>
#include <tuple_field.h>
#include <inet/server.h>

#include <routers/catch.h>
#include <routers/api/types/create/router.h>

// curl -i -G -d "key=val" -d "abs=[1,2,3,4]" http://localhost:36895/api/test

const char *argp_program_version     = "v1.0.0";
const char *argp_program_bug_address = "<pinnecke@ovgu.de>";

struct arguments
{
    size_t port;
};

static struct argp_option options[] =
{
        {"port", 'p', "NUMBER",         0, "Listening to a specific port"},
        {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'p':
            arguments->port = strtoint(arg);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static char args_doc[] = "app.manifest";

static char doc[] =
        "Grid Store -- a fancy database system";

static struct argp argp = {options, parse_opt, args_doc, doc};

//static inline void enter_main_loop(unsigned short port_num);

int main(int argc, char* argv[])
{
    struct arguments arguments;

    arguments.port = 35497;

    argp_parse (&argp, argc, argv, 0, 0, &arguments);
   // enter_main_loop(arguments.port);

    printf("If you see this message, you did good.");

    return EXIT_SUCCESS;
}

/*static inline void enter_main_loop(unsigned short port_num)
{


    server_t server;
    server_create(&server, port_num, NULL);
    server_router_add(&server, "/api/types/create", router_api_types_create);
    server_start(&server, router_catch);
}*/