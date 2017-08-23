#pragma once

#include <stdinc.h>

typedef void (*init_t)(void *element);
typedef void (*inc_t)(void *element);

typedef struct freelist_t {
    inc_t inc;
    void *next_element;
    vector_t *free_elem;
} freelist_t;

void gs_freelist_create(freelist_t *list, size_t elem_size, size_t capacity, init_t init, inc_t inc);
void gs_freelist_free(freelist_t *list);
void gs_freelist_bind(void *out, const freelist_t *list, size_t num_elem);
const void *gs_freelist_peek_new(const freelist_t *list);
void gs_freelist_pushback(freelist_t *list, size_t num_elem, void *elem);