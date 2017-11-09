// An implementation of the double-linked list data structure
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

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct _list gs_list_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

gs_list_t *gs_list_new(size_t element_size);
void gs_list_delete(gs_list_t *list);
bool gs_list_is_empty(const gs_list_t *list);
void gs_list_clear(gs_list_t *list);
bool gs_list_push(gs_list_t *list, const void *data);
const void *gs_list_begin(const gs_list_t *list);
const void *gs_list_next(const void *data);
void gs_list_remove(const void *data);
size_t gs_list_length(const gs_list_t *list);