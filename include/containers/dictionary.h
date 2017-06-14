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
#include <containers/vector.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    dict_type_linear_hash_table
} dict_type_tag;

typedef struct dictionary_t {
    dict_type_tag tag;
    size_t key_size, elem_size;

    void (*clear)(struct dictionary_t *self);
    bool (*empty)(const struct dictionary_t *self);
    bool (*contains_values)(const struct dictionary_t *self, size_t num_values, const void *values);
    bool (*contains_keys)(const struct dictionary_t *self, size_t num_keys, const void *keys);
    const void *(*get)(const struct dictionary_t *self, const void *key);
    vector_t *(*gets)(const struct dictionary_t *self, size_t num_keys, const void *keys);
    bool (*remove)(struct dictionary_t *self, size_t num_keys, const void *keys);
    void (*put)(struct dictionary_t *self, const void *key, const void *value);
    void (*puts)(struct dictionary_t *self, size_t num_elements, const void *keys, const void *values);
    size_t (*num_elements)(struct dictionary_t *self);
    void (*for_each)(struct dictionary_t *self, void *capture, void (*consumer)(void *capture, const void *key, const void *value));

    void *extra;
} dictionary_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void dictionary_clear(dictionary_t *dictionary);
bool dictionary_empty(const dictionary_t *dictionary);
bool dictionary_contains_values(const dictionary_t *dictionary, size_t num_values, const void *values);
bool dictionary_contains_keys(const dictionary_t *dictionary, size_t num_keys, const void *keys);
const void *dictionary_get(const dictionary_t *dictionary, const void *key);
vector_t *dictionary_gets(const dictionary_t *dictionary, size_t num_keys, const void *keys);
bool dictionary_remove(dictionary_t *dictionary, size_t num_keys, const void *keys);
void dictionary_put(dictionary_t *dictionary, const void *key, const void *value);
void dictionary_puts(dictionary_t *dictionary, size_t num_elements, const void *keys, const void *values);
size_t dictionary_num_elements(dictionary_t *dictionary);
void dictionary_for_each(dictionary_t *dictionary, void *capture, void (*consumer)(void *capture, const void *key, const void *value));