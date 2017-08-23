#pragma once

#include <stdinc.h>
#include <tuple.h>

typedef struct interval_t {
    size_t begin;
    size_t end;
} interval_t;

typedef struct tuple_id_interval_t {
    tuple_id_t begin;
    tuple_id_t end;
} tuple_id_interval_t;

#define gs_interval_get_span(interval)                   \
    ({                                                   \
        (interval->end - interval->begin);               \
    })