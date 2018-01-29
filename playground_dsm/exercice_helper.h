#pragma once

#include <gs_frag.h>


#define TIMER_STOP()                                                            \
({                                                                              \
    struct timeval timeval;                                                     \
    gettimeofday(&timeval, NULL);                                               \
    long start = (long)timeval.tv_sec * 1000 + (long)timeval.tv_usec / 1000;    \
    start;                                                                      \
})

gs_frag_t *help_create_lineitem_table(size_t capacity,size_t num_fields,enum gs_frag_impl_type_e type);
void help_fill_lineitem_table(gs_frag_t *frag,size_t num_tuples);

static inline int exercice_cmp_int32(const void *lhs, const void *rhs) { return *(int32_t *)lhs - *(int32_t *)rhs; }
static inline int exercice_cmp_uint32(const void *lhs, const void *rhs) { return (int64_t)*(uint32_t *)lhs - (int64_t)*(uint32_t *)rhs; }
static inline int exercice_cmp_uint64(const void *lhs, const void *rhs) { return ((int64_t)*(uint64_t *)lhs) - ((int64_t)*(uint64_t *)rhs); }
static inline int exercice_cmp_float32(const void *lhs, const void *rhs) { return (1000 * *(float *)lhs) - (1000 * *(float *)rhs); } /* super crappy! */
static inline int exercice_cmp_char(const void *lhs, const void *rhs) { return strcmp((char *)lhs, (char *)rhs); }

typedef enum exercice_pred_e {
    exercice_pred_less, exercice_pred_greater, exercice_pred_equals, exercice_pred_lesseq,
    exercice_pred_greatereq,exercice_pred_unequal
} exercice_pred_e;

typedef int (*exercice_cmp_func_t)(const void*, const void *);
