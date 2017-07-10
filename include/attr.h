#pragma once

#include <stdinc.h>

typedef struct {
    size_t id;
    char name[_ATTR_NUMMAX];
    enum field_type type;
    size_t type_rep;
    ATTR_FLAGS flags;
    size_t foreign_id;
    size_t str_format_mlen;
    unsigned char checksum[MD5_DIGEST_LENGTH];
} attr_t;



