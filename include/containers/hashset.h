#pragma once

#include <stdinc.h>

typedef struct hashset_t
{
    dict_t *dict;
    vector_t *vec;
} hashset_t;

void gs_hashset_create(hashset_t *out, size_t elem_size, size_t capacity);

void gs_hashset_free(hashset_t *set);

void gs_hashset_add(hashset_t *set, const void *data, size_t num_elems);

void gs_hashset_remove(hashset_t *set, const void *data, size_t num_elems);

bool gs_hashset_contains(const hashset_t *set, const void *data);

const void *gs_hashset_begin(const hashset_t *set);

const void *gs_hashset_end(const hashset_t *set);