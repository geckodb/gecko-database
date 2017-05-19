#pragma once

#include <defs.h>

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
    err_constraint_violated
} error_code;

void error(error_code code);

void error_if(bool expr, error_code code);

error_code error_last();

const char *error_str(error_code code);

void error_print();