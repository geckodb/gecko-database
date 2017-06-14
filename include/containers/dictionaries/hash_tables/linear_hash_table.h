// An implementation of the linear hash table data structure
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
#include <containers/dictionaries/hash_table.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef void (*linear_hash_table_dtor_t)(void *self);

typedef struct {
    struct {
        struct {
            hash_table_t base;
            vector_t *data;
        } member;
    } protected;
    struct {
        struct {
            linear_hash_table_dtor_t destruct;
        } methods;
    } public;
} linear_hash_table_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void linear_hash_table_construct(linear_hash_table_t *table, hash_code_fn_t hash_code_fn,
                                 hash_fn_t hash_fn, size_t key_size, size_t elem_size);
