// An implementation of the vector data structure
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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

#include <stdinc.h>

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

typedef struct vector_t {
    size_t sizeof_element, num_elements, element_capacity;
    vector_flags flags;
    float grow_factor;
    void *data;
    bool is_sorted;
} vector_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

vector_t *vector_create(size_t element_size, size_t capacity);

vector_t *vector_create_ex(size_t element_size, size_t capacity, vector_flags flags, float grow_factor);

bool vector_resize(vector_t *vec, size_t num_elements);

bool vector_reserve(vector_t *vec, size_t num_elements);

size_t vector_num_elements(const vector_t *vec);

void vector_memset(vector_t *vec, size_t pos_start, size_t num_elements, const void *data);

vector_t *vector_cpy(vector_t *proto);

bool vector_free(struct vector_t *vec);

bool vector_free_ex(vector_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end));

void *vector_get(vector_t *vec);

void *vector_at(const vector_t *vec, size_t pos);

void *vector_peek(const vector_t *vec);

void *vector_begin(const vector_t *vec);

void *vector_end(const vector_t *vec);

bool vector_issorted(vector_t *vec, cache_consideration_policy policy, comp_t comp);

bool vector_updatesort(vector_t *vec, comp_t comp);

void *vector_pop_unsafe(vector_t *vec);

void *vector_peek_unsafe(vector_t *vec);

bool vector_set(vector_t *vec, size_t idx, size_t num_elements, const void *data);

bool vector_add(vector_t *vec, size_t num_elements, const void *data);

bool vector_add_all(vector_t *dest, const vector_t *src);

bool vector_add_all_unsafe(vector_t *dest, const vector_t *src);

void vector_add_unsafe(vector_t *vec, size_t num_elements, const void *data);

bool vector_comp(const vector_t *lhs, const vector_t *rhs, comp_t comp);

bool vector_foreach(vector_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end));

size_t vector_count(vector_t *vec, void *capture, bool (*pred)(void *capture, void *it));

bool vector_contains(vector_t *vec, void *needle);

size_t vector_memused(vector_t *vec);

size_t vector_memused__str(vector_t *vec);

size_t vector_sizeof(const vector_t *vec);

void vector_sort(vector_t *vec, comp_t comp);

void *vector_bsearch(vector_t *vec, const void *needle, comp_t sort_comp, comp_t find_comp);

// ---------------------------------------------------------------------------------------------------------------------
// C O N V E N I E N C E  F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool vector_free__str(vector_t *vec);

bool free_strings(void *capture, void *begin, void *end);

bool get_sizeof_strings(void *capture, void *begin, void *end);