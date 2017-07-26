
#include <stdinc.h>
#include <stdlib.h>
#include <printf.h>
#include <memory.h>
#include <shell.h>
#include <inet/server.h>
#include <conf.h>
#include <debug.h>

#define CMD_INIT            "init"
#define CMD_START           "start"
#define CMD_SHELL           "shell"

#define CMD_VERSION         "--version"
#define CMD_VERSION2        "-v"

#define CMD_HELP            "--help"
#define CMD_HELP2           "-h"

static void show_usage();
static void show_version();

static void start_server(uint32_t port, const char *db_path);
static void start_shell(const char *connection_str);
static void init_db_path(const char *db_path);

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        goto print_usage_infos;
    } else {
        const char *command = argv[1];
        gs_db_config_load(&G_DB_CONFIG);

        if (argc == 2) {
            if (!strcmp(command, CMD_VERSION) || !strcmp(command, CMD_VERSION2)) {
                show_version();
            } else {
                goto print_usage_infos;
            }
        } else if (argc == 3) {
            if (!strcmp(command, CMD_SHELL)) {
                start_shell(argv[2]);
            } else if (!strcmp(command, CMD_INIT)) {
                init_db_path(argv[2]);
            } else if (!strcmp(command, CMD_START)) {
                start_server(G_DB_CONFIG.inet.port, argv[2]);
            }
        } else if (argc == 4) {
            start_server(atoi(argv[2]), argv[3]);
        } else {
print_usage_infos:
            show_usage();
        }
    }

    return EXIT_SUCCESS;
}

static void show_version()
{
    printf("MondrianDB 0.00 (%s)\n", __DATE__);
    printf("v0.0.0-0001-vanilla, Copyright (c) 2017 Marcus Pinnecke. All rights reserved.\n");
}


static void show_usage()
{
    printf("Usage: gs [command]\n\n");
    printf("Commands:\n");
    printf("  init <path>                    initializes <path> as database directory\n");
    printf("  start [port <port>] <path>     starts server in <path> database directory,\n"
           "                                 binds to port <port> (default is 1316)\n");
    printf("  shell <address>:<port>         connects to server at <address>:<port> and\n"
                                             "starts terminal shell\n");
    printf("  -v, --version                  print MondrianDB version\n");
    printf("  -h, --help                     print usage information\n\n");
    printf("Environment variables:\n");
    printf("MONDRIAN_HOME      ':'-separated list of directories that are prefixed to the\n"
           "                   path search.\n");
}

static void start_server(uint32_t port, const char *db_path)
{
    LOG_DEBUG("starting server, port:%d, database:'%s'", port, db_path);

    require_non_null(db_path);
    G_DB_CONFIG.inet.port = port;

    server_t server;
    gs_server_create(&server, htons(G_DB_CONFIG.inet.port));
    gs_server_start(&server);
}

static void start_shell(const char *connection_str)
{
    LOG_DEBUG("starting shell, connect_to:'%s'", connection_str);

    require_non_null(connection_str);
    gs_shell_start(connection_str);
}

static void init_db_path(const char *db_path)
{
    LOG_DEBUG("init database, path:'%s'", db_path);

    require_non_null(db_path);
}