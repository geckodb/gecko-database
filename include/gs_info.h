// Several pre-defined functions related to hash operations
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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

const char *argp_program_version     = "v1.0.0";
const char *argp_program_bug_address = "<pinnecke@ovgu.de>";

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_startup_config_t {
    size_t port;
} gs_startup_config_t;

gs_startup_config_t gs_startup_config;

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
    gs_startup_config_t *config = state->input;

    switch (key) {
        case 'p':
            config->port = gs_strtoint(arg);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}