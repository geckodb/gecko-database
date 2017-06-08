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

#define panic(msg, args...)                                                                                            \
    {                                                                                                                  \
        fflush(stdout);                                                                                                \
        fflush(stderr);                                                                                                \
        fprintf(stderr, "# \n");                                                                                       \
        trace_print(stderr);                                                                                           \
        fprintf(stderr, "# \n");                                                                                       \
        fprintf(stderr, "# Core panic (%s:%d): ", __FILE__, __LINE__);                                                 \
        fprintf(stderr, msg, args);                                                                                    \
        fflush(stderr);                                                                                                \
        exit(1);                                                                                                       \
    }

#define panic_if(expr, msg, args...)                                                                                   \
    {                                                                                                                  \
        if (expr) { panic(msg, args); }                                                                                \
    }

void error_if(bool expr, error_code code);

error_code error_last();

const char *error_str(error_code code);

void error_print();