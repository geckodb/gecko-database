#pragma once

#include <defs.h>

enum error_code
{
    EC_NOERR,
    EC_ILLEGALARG,
    EC_BADMALLOC,
    EC_NULLPOINTER,
    EC_BADREALLOC,
    EC_ILLEGALOPP,
    EC_UNKNOWNTYPE,
    EC_OUTOFBOUNDS,
    EC_CORRUPTEDORDER,
    EC_INTERNALERROR,
    EC_RELATIONVIOLATED
};

void error_set_last(enum error_code code);

void error_set_last_if(bool expr, enum error_code code);

enum error_code error_get_last();

const char *error_to_str(enum error_code code);

void error_print();