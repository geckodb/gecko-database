#pragma once

#include <stdinc.h>

enum field_type {
    FT_BOOL,
    FT_INT8,
    FT_INT16,
    FT_INT32,
    FT_INT64,
    FT_UINT8,
    FT_UINT16,
    FT_UINT32,
    FT_UINT64,
    FT_FLOAT32,
    FT_FLOAT64,
    FT_CHAR
};

size_t gs_field_type_sizeof(enum field_type type);