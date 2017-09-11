#pragma once

#include <stdinc.h>

enum field_type {
    FT_BOOL     =  0,
    FT_INT8     =  1,
    FT_INT16    =  2,
    FT_INT32    =  3,
    FT_INT64    =  4,
    FT_UINT8    =  5,
    FT_UINT16   =  6,
    FT_UINT32   =  7,
    FT_UINT64   =  8,
    FT_FLOAT32  =  9,
    FT_FLOAT64  = 10,
    FT_CHAR     = 11,
    /* internal */
    FT_STRPTR   = 100,
    FT_ATTRID   = 101,
};

size_t gs_field_type_sizeof(enum field_type type);

const char *gs_field_type_str(enum field_type type);