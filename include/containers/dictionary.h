// An implementation of the dictionary abstract data structure
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
#include <object.h>
#include <containers/vector.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

struct dictionary_t;

typedef bool (*dictionary_clear_fn_t)(void *self);
typedef bool (*dictionary_empty_fn_t)(const void *self);
typedef bool (*dictionary_contains_values_fn_t)(const void *self, size_t num_values, const void *values);
typedef bool (*dictionary_contains_keys_fn_t)(const void *self, size_t num_keys, const void *keys);
typedef const void *(*dictionary_get_fn_t)(const void *self, const void *key);
typedef vector_t *(*dictionary_gets_fn_t)(const void *self, size_t num_keys, const void *keys);
typedef bool (*dictionary_remove_fn_t)(void *self, size_t num_keys, const void *keys);
typedef bool (*dictionary_put_fn_t)(void *self, const void *key, const void *value);
typedef size_t (*dictionary_num_elements_fn_t)(void *self);

typedef struct dictionary_t {
    struct {
        struct {
            object_t base;
            size_t elem_size, key_size;
        } members;
    } protected;

    struct {
        struct {
            dictionary_clear_fn_t clear;
            dictionary_empty_fn_t is_empty;
            dictionary_contains_values_fn_t contains_values;
            dictionary_contains_keys_fn_t contains_keys;
            dictionary_get_fn_t get;
            dictionary_gets_fn_t gets;
            dictionary_remove_fn_t remove;
            dictionary_put_fn_t put;
            dictionary_num_elements_fn_t num_elements;
        } methods;
    } public;
} dictionary_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void dictionary_create(dictionary_t *dictionary, size_t key_size, size_t elem_size,
                       dictionary_clear_fn_t clear, dictionary_empty_fn_t empty,
                       dictionary_contains_values_fn_t contains_values, dictionary_contains_keys_fn_t contains_keys,
                       dictionary_get_fn_t get, dictionary_gets_fn_t gets, dictionary_remove_fn_t remove,
                       dictionary_put_fn_t put, dictionary_num_elements_fn_t num_elements);

void dictionary_override_to_string(dictionary_t *dictionary, object_to_string_fn_t f);

