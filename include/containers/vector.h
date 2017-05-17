#pragma once

#include <defs.h>

#define GROW_FACTOR 1.7

struct vector;

enum vector_flags {
    VF_ZERO_MEMORY = 1 << 0,
    VF_AUTO_RESIZE = 1 << 1,
};

struct vector *vector_create(size_t element_size, size_t capacity);

struct vector *vector_create_ex(size_t element_size, size_t capacity, enum vector_flags flags, float grow_factor);

const void *vector_get_data(struct vector *vec);

bool vector_set(struct vector *vec, size_t idx, size_t num_elements, const void *data);

size_t vector_get_num_elements(struct vector *vec);

size_t vector_get_elements_size(struct vector *vec);

bool vector_push_back(struct vector *vec, size_t num_elements, const void *data);

bool vector_delete(struct vector *vec);

bool vector_equals(const struct vector *lhs, const struct vector *rhs, bool (*comp)(const void *a, const void *b));

bool vector_foreach(struct vector *vec, bool (*consumer)(void *begin, void *end));