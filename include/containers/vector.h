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

#include <defs.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define GROW_FACTOR 1.7

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum vector_flags {
    VF_ZERO_MEMORY = 1 << 0,
    VF_AUTO_RESIZE = 1 << 1,
};

typedef struct
{
    void *data;
    size_t sizeof_element, num_elements, element_capacity;
    enum vector_flags flags;
    float grow_factor;
} mdb_vector;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

mdb_vector *mdb_vector_alloc(size_t element_size, size_t capacity);

mdb_vector *mdb_vector_alloc_ex(size_t element_size, size_t capacity, enum vector_flags flags, float grow_factor);

bool mdb_vector_free(mdb_vector *vec);

const void *mdb_vector_get(mdb_vector *vec);

bool mdb_vector_set(mdb_vector *vec, size_t idx, size_t num_elements, const void *data);

bool mdb_vector_add(mdb_vector *vec, size_t num_elements, const void *data);

bool mdb_vector_comp(const mdb_vector *lhs, const mdb_vector *rhs, bool (*comp)(const void *a, const void *b));

bool mdb_vector_foreach(mdb_vector *vec, void *capture, bool (*func)(void *capture, void *begin, void *end));