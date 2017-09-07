#pragma once

#include "common.h"
#include "types.h"

static inline uint64_t to_uint64(const char* str)
{
    uint64_t fact   = 1;
    uint64_t result = 0;
    int i = strlen(str) - 1;

    while (i--) {
        const char c = str[i];
        if (c == '-') {
            result *= -1;
        }
        if ((c == '0')|| (c == '1')|| (c == '2')|| (c == '3')|| (c == '4')|| (c == '5')||
            (c == '6')|| (c == '7')|| (c == '8')|| (c == '9')) {
            result = result + ((int)c - 48) * fact;
            fact *= 10;
        }
    }
    return result;
}

static inline void identifer(tpch_identifier_t *dst, const char *src)
{
    *dst = (tpch_identifier_t) to_uint64(src);
}

static inline void decimal(tpch_decimal_t *dst, const char *src)
{
    *dst = (tpch_decimal_t) to_uint64(src);
}

static inline void text(tpch_text_t *dst, const char *src, size_t n)
{
    *dst = require_good_malloc(n);
    strcpy(*dst, src);
}



