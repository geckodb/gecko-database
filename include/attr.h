#pragma once

#include <stdinc.h>

typedef struct {
    size_t id;
    char name[ATTR_NAME_MAXLEN];
    enum field_type type;
    size_t type_rep;
    ATTR_FLAGS flags;
    size_t foreign_id;
    size_t str_format_mlen;
    unsigned char checksum[MD5_DIGEST_LENGTH];
} attr_t;

const char *gs_attr_get_name(attr_t *attr);

size_t gs_attr_get_str_format_max_len(attr_t *attr);

enum field_type gs_attr_get_type(const attr_t *attr);


