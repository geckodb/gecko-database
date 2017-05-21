#pragma once

#include <defs.h>

typedef struct {
    void *prev, *next;
    void *data;
} list_node_t;

typedef struct {
    list_node_t *root, *tail;
    size_t num_elements, element_size;
} list_t;

list_t *list_create(size_t element_size);

bool list_free(list_t *list);

bool list_is_empty(const list_t *list);

bool list_clear(list_t *list);

bool list_push(list_t *list, const void *data);

const void *list_begin(const list_t *list);

const void *list_next(const void *data);

bool list_remove(list_t *list, const void *data);

size_t list_num_elements(const list_t *list);