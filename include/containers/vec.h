// An implementation of the vector data structure
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs.h>
#include <apr_pools.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define GROW_FACTOR 1.7

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef int (*comp_t)(const void *lhs, const void *rhs);

typedef enum {
    CCP_IGNORECACHE,
    CCP_USECACHE
} cache_consideration_policy;

typedef enum {
    zero_memory = 1 << 0,
    auto_resize = 1 << 1,
} vector_flags;

typedef struct vec_t {
    size_t sizeof_element, num_elements, element_capacity;
    vector_flags flags;
    float grow_factor;
    void *data;
    bool is_sorted;
    apr_pool_t *pool;
} vec_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

vec_t *vec_new(size_t element_size, size_t capacity);
vec_t *vec_new_ex(size_t element_size, size_t capacity, vector_flags flags, float grow_factor);
void vec_free(struct vec_t *vec);
void vec_dispose(struct vec_t *vec);
bool vec_resize(vec_t *vec, size_t num_elements);
bool vec_reserve(vec_t *vec, size_t num_elements);
size_t vec_length(const vec_t *vec);
void vec_memset(vec_t *vec, size_t pos_start, size_t num_elements, const void *data);
vec_t *vec_cpy_deep(vec_t *proto);
void vec_cpy_shallow(vec_t *dst, vec_t *src);
void vec_free_ex(vec_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end));
void *vec_data(vec_t *vec);
void *vec_at(const vec_t *vec, size_t pos);
void *vec_peek(const vec_t *vec);
void *vec_begin(const vec_t *vec);
void *vec_end(const vec_t *vec);
bool vec_issorted(vec_t *vec, cache_consideration_policy policy, comp_t comp);
bool vec_updatesort(vec_t *vec, comp_t comp);
void *vec_pop_unsafe(vec_t *vec);
void *vec_peek_unsafe(vec_t *vec);
bool vec_set(vec_t *vec, size_t idx, size_t num_elements, const void *data);
bool vec_pushback(vec_t *vec, size_t num_elements, const void *data);
bool vec_add_all(vec_t *dest, const vec_t *src);
bool vec_add_all_unsafe(vec_t *dest, const vec_t *src);
void vec_add_unsafe(vec_t *vec, size_t num_elements, const void *data);
bool vec_comp(const vec_t *lhs, const vec_t *rhs, comp_t comp);
bool vec_foreach(vec_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end));
void vec_swap(vec_t *lhs, vec_t *rhs);
size_t vec_count(vec_t *vec, void *capture, bool (*pred)(void *capture, void *it));
bool vec_contains(vec_t *vec, void *needle);
size_t vec_memused(vec_t *vec);
size_t vec_memused__str(vec_t *vec);
size_t vec_sizeof(const vec_t *vec);
void vec_sort(vec_t *vec, comp_t comp);
void *vec_bsearch(vec_t *vec, const void *needle, comp_t sort_comp, comp_t find_comp);

// ---------------------------------------------------------------------------------------------------------------------
// C O N V E N I E N C E  F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void vec_free__str(vec_t *vec);
bool free_strings(void *capture, void *begin, void *end);
bool get_sizeof_strings(void *capture, void *begin, void *end);