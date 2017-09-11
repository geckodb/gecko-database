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

#define gs_interval_get_span(interval)                                   \
    ({                                                                   \
        (interval->end - interval->begin);                               \
    })

#define GS_INTERVAL_CONTAINS(interval, elem)                             \
    ({                                                                   \
        (elem >= interval->begin && elem < interval->end);               \
    })

#define gs_interval_equals(a, b)                                         \
    ({                                                                   \
        (a->begin == b->begin && a->end == b->end);                      \
    })

static inline int gs_interval_tuple_id_comp_by_lower_bound(const void *lhs, const void *rhs)
{
    const tuple_id_interval_t *a = lhs;
    const tuple_id_interval_t *b = rhs;
    return (a->begin < b->begin ? - 1 : (a->begin > b->begin ? + 1 : 0));
}