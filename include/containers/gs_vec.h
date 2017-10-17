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

typedef int (*gs_comp_t)(const void *lhs, const void *rhs);

typedef enum {
    CCP_IGNORECACHE,
    CCP_USECACHE
} gs_cache_consideration_policy_e;

typedef enum {
    zero_memory = 1 << 0,
    auto_resize = 1 << 1,
} gs_vector_flags_e;

typedef struct gs_vec_t {
    size_t sizeof_element, num_elements, element_capacity;
    gs_vector_flags_e flags;
    float grow_factor;
    void *data;
    bool is_sorted;
    apr_pool_t *pool;
} gs_vec_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

gs_vec_t *gs_vec_new(size_t element_size, size_t capacity);
gs_vec_t *gs_vec_new_ex(size_t element_size, size_t capacity, gs_vector_flags_e flags, float grow_factor);
void gs_vec_free(struct gs_vec_t *vec);
void gs_vec_dispose(struct gs_vec_t *vec);
bool gs_vec_resize(gs_vec_t *vec, size_t num_elements);
bool gs_vec_reserve(gs_vec_t *vec, size_t num_elements);
size_t gs_vec_length(const gs_vec_t *vec);
void gs_vec_memset(gs_vec_t *vec, size_t pos_start, size_t num_elements, const void *data);
gs_vec_t *gs_vec_cpy_deep(gs_vec_t *proto);
void gs_vec_cpy_shallow(gs_vec_t *dst, gs_vec_t *src);
void gs_vec_free_ex(gs_vec_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end));
void *gs_vec_data(gs_vec_t *vec);
void *gs_vec_at(const gs_vec_t *vec, size_t pos);
void *gs_vec_peek(const gs_vec_t *vec);
void *gs_vec_begin(const gs_vec_t *vec);
void *gs_vec_end(const gs_vec_t *vec);
bool gs_vec_issorted(gs_vec_t *vec, gs_cache_consideration_policy_e policy, gs_comp_t comp);
bool gs_vec_updatesort(gs_vec_t *vec, gs_comp_t comp);
void *gs_vec_pop_unsafe(gs_vec_t *vec);
void *gs_vec_peek_unsafe(gs_vec_t *vec);
bool gs_vec_set(gs_vec_t *vec, size_t idx, size_t num_elements, const void *data);
bool gs_vec_pushback(gs_vec_t *vec, size_t num_elements, const void *data);
bool gs_vec_add_all(gs_vec_t *dest, const gs_vec_t *src);
bool gs_vec_add_all_unsafe(gs_vec_t *dest, const gs_vec_t *src);
void gs_vec_add_unsafe(gs_vec_t *vec, size_t num_elements, const void *data);
bool gs_vec_comp(const gs_vec_t *lhs, const gs_vec_t *rhs, gs_comp_t comp);
bool gs_vec_foreach(gs_vec_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end));
void gs_vec_swap(gs_vec_t *lhs, gs_vec_t *rhs);
size_t gs_vec_count(gs_vec_t *vec, void *capture, bool (*pred)(void *capture, void *it));
bool gs_vec_contains(gs_vec_t *vec, void *needle);
size_t gs_vec_memused(gs_vec_t *vec);
size_t gs_vec_memused__str(gs_vec_t *vec);
size_t gs_vec_sizeof(const gs_vec_t *vec);
void gs_vec_sort(gs_vec_t *vec, gs_comp_t comp);
void *gs_vec_bsearch(gs_vec_t *vec, const void *needle, gs_comp_t sort_comp, gs_comp_t find_comp);

// ---------------------------------------------------------------------------------------------------------------------
// C O N V E N I E N C E  F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void gs_vec_free__str(gs_vec_t *vec);
bool gs_free_strings(void *capture, void *begin, void *end);
bool gs_get_sizeof_strings(void *capture, void *begin, void *end);