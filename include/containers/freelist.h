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
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef void (*init_t)(void *element);
typedef void (*inc_t)(void *element);

typedef struct freelist_t {
    inc_t inc;
    void *next_element;
    vec_t *free_elem;
} freelist_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void gs_freelist_create(freelist_t *list, size_t elem_size, size_t capacity, init_t init, inc_t inc);
void gs_freelist_free(freelist_t *list);
void gs_freelist_bind(void *out, const freelist_t *list, size_t num_elem);
const void *gs_freelist_peek_new(const freelist_t *list);
void gs_freelist_pushback(freelist_t *list, size_t num_elem, void *elem);