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
#include "gs_vec.h"

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef void (*gs_init_t)(void *element);
typedef void (*gs_inc_t)(void *element);

typedef struct gs_freelist_t {
    gs_inc_t inc;
    void *next_element;
    gs_vec_t *free_elem;
} gs_freelist_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void gs_freelist_create(gs_freelist_t *list, size_t elem_size, size_t capacity, gs_init_t init, gs_inc_t inc);
void gs_freelist_create2(gs_freelist_t **list, size_t elem_size, size_t capacity, gs_init_t init, gs_inc_t inc);
void gs_freelist_dispose(gs_freelist_t *list);
void gs_freelist_free(gs_freelist_t *list);
void gs_freelist_bind(void *out, const gs_freelist_t *list, size_t num_elem);
const void *gs_freelist_peek_new(const gs_freelist_t *list);
void gs_freelist_pushback(gs_freelist_t *list, size_t num_elem, void *elem);