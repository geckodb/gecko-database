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

#include <stdinc.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct hashset_t
{
    dict_t *dict;
    vec_t *vec;
} hashset_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void hashset_create(hashset_t *out, size_t elem_size, size_t capacity);
void hashset_dispose(hashset_t *set);
void hashset_add(hashset_t *set, const void *data, size_t num_elems);
void hashset_remove(hashset_t *set, const void *data, size_t num_elems);
bool hashset_contains(const hashset_t *set, const void *data);
const void *hashset_begin(const hashset_t *set);
const void *hashset_end(const hashset_t *set);