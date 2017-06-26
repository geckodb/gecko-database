#pragma once

#include <defs.h>
#include <execinfo.h>

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
        begin_write("WARNING");                                                                                        \
        fprintf(stderr, msg, args);                                                                                    \
        end_write();                                                                                                   \
    }

#define panic_if(expr, msg, args...)                                                                                   \
    {                                                                                                                  \
        if (expr) { panic(msg, args); }                                                                                \
    }

#define warn_if(expr, msg, args...)                                                                                    \
    {                                                                                                                  \
        if (expr) { warn(msg, args); }                                                                                 \
    }

void error_if(bool expr, error_code code);

const char *error_str(error_code code);

void error_print();