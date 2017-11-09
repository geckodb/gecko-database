#pragma once

#include <argp.h>

const char *argp_program_version     = "v1.0.0";
const char *argp_program_bug_address = "<pinnecke@ovgu.de>";

typedef struct startup_config_t {
    size_t port;
} startup_config_t;

startup_config_t startup_config;

static struct argp_option options[] = {
        {"port", 'p', "NUMBER",         0, "Listening to a specific port"},
        {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);

static char args_doc[] = "app.manifest";

static char doc[] = "Grid Store -- a fancy database system";

static struct argp argp = {options, parse_opt, args_doc, doc};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    startup_config_t *config = state->input;

    switch (key) {
        case 'p':
            config->port = gs_strtoint(arg);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}