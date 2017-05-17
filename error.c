#include <error.h>

enum error_code last_error;

void error_set_last(enum error_code code)
{
    last_error = code;
}

void error_set_last_if(bool expr, enum error_code code)
{
    if (__builtin_expect(expr, false))
        error_set_last(code);
}

enum error_code error_get_last()
{
    return last_error;
}

const char *error_to_str(enum error_code code)
{
    switch (code) {
        case EC_ILLEGALARG:     return "Illegal argument";
        case EC_BADMALLOC:      return "Host memory allocation failed";
        case EC_NULLPOINTER:    return "Pointer to null";
        case EC_BADREALLOC:     return "Host memory reallocation failed";
        case EC_ILLEGALOPP:     return "Illegal operation";
        case EC_UNKNOWNTYPE:    return "Unknown internal type";
        case EC_OUTOFBOUNDS:    return "Out of bounds";
        case EC_NOERR:          return "No error description";
        case EC_CORRUPTEDORDER: return "Corrupted order";
        default: return "Unknown";
    }
}

void error_print()
{
    int last = error_get_last();
    fprintf(stderr, "ERROR 0x%08x: %s\n", last, error_to_str(last));
}