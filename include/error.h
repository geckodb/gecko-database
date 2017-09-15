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
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdinc.h>

#define MONDRIAN_OK                 1
#define MONDRIAN_CONTINUE           1
#define MONDRIAN_BREAK              0
#define MONDRIAN_ERROR              0
#define MONDRIAN_NOSUCHELEM         2
#define MONDRIAN_ALREADY_DONE   3

#define MONDRIAN_VM_COMMIT          0
#define MONDRIAN_VM_USER_ABORT      1
#define MONDRIAN_VM_SYSTEM_ABORT    2

typedef enum {
    err_no_error,
    err_illegal_args,
    err_bad_malloc,
    err_null_ptr,
    err_bad_realloc,
    err_illegal_opp,
    err_bad_type,
    err_out_of_bounds,
    err_corrupted,
    err_internal,
    err_constraint_violated,
    err_limitreached,
    err_no_free_space,
    err_notincharge
} error_code;

void error(error_code code);

void error_reset();

error_code error_get();

void trace_print(FILE *file);

#define begin_write(caption)                                                                                           \
        fflush(stdout);                                                                                                \
        fflush(stderr);                                                                                                \
        fprintf(stderr, "# \n");                                                                                       \
        trace_print(stderr);                                                                                           \
        fprintf(stderr, "# \n");                                                                                       \
        fprintf(stderr, "# " caption " ('%s:%d'): ", __FILE__, __LINE__);

#define end_write()                                                                                                    \
        fprintf(stderr, "\n");                                                                                         \
        fflush(stderr);

#define panic(msg, args...)                                                                                            \
    {                                                                                                                  \
        begin_write("Core panic");                                                                                     \
        fprintf(stderr, msg, args);                                                                                    \
        end_write();                                                                                                   \
        abort();                                                                                                       \
    }

#define warn(msg, args...)                                                                                             \
    {                                                                                                                  \
        /*begin_write("WARNING");                                                                                      \
        fprintf(stderr, msg, args);                                                                                    \
        end_write();  */                                                                                               \
    }

#define panic_if(expr, msg, args...)                                                                                   \
    {                                                                                                                  \
        if (expr) { panic(msg, args); }                                                                                \
    }

#define WARN_IF(expr, msg, args...)                                                                                    \
    {                                                                                                                  \
        if (expr) { warn(msg, args); }                                                                                 \
    }

void error_if(bool expr, error_code code);

const char *error_str(error_code code);

void error_print();