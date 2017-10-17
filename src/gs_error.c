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

#include <gs_error.h>

error_code last_error;

void error(error_code code)
{
    panic("ERROR %d", code);
    last_error = code;
}

void error_reset()
{
    last_error = err_no_error;
}

error_code error_get()
{
    return last_error;
}

void trace_print(FILE *file)
{
    fflush(stdout);
    fflush(stderr);
    void *array[10];
    size_t size = backtrace (array, 10);
    char **strings = backtrace_symbols (array, size);
    fprintf (file, "# Trace %zd stack frames:\n", size);
    for (size_t i = 1; i < size - 1; i++)
    fprintf (file, "# [%s]\n", strings[i]);
    free (strings);
    fflush(file);
}

void error_if(bool expr, error_code code)
{
    if (__builtin_expect(expr, false))
        error(code);
}

const char *error_str(error_code code)
{
    switch (code) {
        case err_illegal_args:           return "Illegal argument";
        case err_bad_malloc:             return "Host memory allocation failed";
        case err_null_ptr:               return "Pointer to null";
        case err_bad_realloc:            return "Host memory reallocation failed";
        case err_illegal_opp:            return "Illegal operation";
        case err_bad_type:               return "Unknown internal type";
        case err_out_of_bounds:          return "Out of bounds";
        case err_no_error:               return "No error description";
        case err_corrupted:              return "Corrupted order";
        case err_internal:               return "Internal error";
        case err_constraint_violated:    return "Constraint violated";
        case err_limitreached:           return "Limit reached";
        case err_no_free_space:          return "No space available";
        case err_notincharge:            return "Request was rejected: not in charge";
        case err_dispatcher_terminated:  return "Event dispatcher has been terminated unexpectedly";
        case err_apr_initfailed:         return "apr initialization failed";
        case err_no_stdin:               return "unable to open stdin";
        default: return "Unknown";
    }
}

void error_print()
{
    int last = error_get();
    fprintf(stderr, "ERROR 0x%08x: %s\n", last, error_str(last));
}