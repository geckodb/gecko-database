// An implementation of the hash table data structure
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
#include <containers/dictionary.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef size_t (*hash_code_fn_t)(const void *data);
typedef size_t (*hash_fn_t)(size_t code, size_t upper_bound);

typedef struct {
    struct {
        struct {
            dictionary_t base;
        } members;
        struct {
            hash_code_fn_t hash_code_fn;
            hash_fn_t hash_fn;
        } methods;
    } protected;
} hash_table_t;

// ---------------------------------------------------------------------------------------------------------------------
// H A S H   C O D E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t hash_code_size_t(const void *data);

// ---------------------------------------------------------------------------------------------------------------------
// H A S H   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t hash_fn_mod(size_t code, size_t upper_bound);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void hash_table_create(hash_table_t *table, hash_code_fn_t hash_code_fn, hash_fn_t hash_fn, size_t key_size,
                       size_t elem_size, dictionary_clear_fn_t clear, dictionary_empty_fn_t empty,
                       dictionary_contains_values_fn_t contains_values, dictionary_contains_keys_fn_t contains_keys,
                       dictionary_get_fn_t get, dictionary_gets_fn_t gets, dictionary_remove_fn_t remove,
                       dictionary_put_fn_t put, dictionary_num_elements_fn_t num_elements);

void hash_table_override_to_string(hash_table_t *table, object_to_string_fn_t f);


