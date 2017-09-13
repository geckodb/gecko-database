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

#include <stdinc.h>
#include <containers/vec.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    dict_type_linear_hash_table
} dict_type_tag;

typedef struct dict_t {
    dict_type_tag tag;
    size_t key_size, elem_size;

    bool (*free_dict)(struct dict_t *self);
    void (*clear)(struct dict_t *self);
    bool (*empty)(const struct dict_t *self);
    bool (*contains_key)(const struct dict_t *self, const void *key);
    const struct vec_t *(*keyset)(const struct dict_t *self);
    const void *(*get)(const struct dict_t *self, const void *key);
    struct vec_t *(*gets)(const struct dict_t *self, size_t num_keys, const void *keys);
    bool (*remove)(struct dict_t *self, size_t num_keys, const void *keys);
    void (*put)(struct dict_t *self, const void *key, const void *value);
    void (*puts)(struct dict_t *self, size_t num_elements, const void *keys, const void *values);
    size_t (*num_elements)(struct dict_t *self);
    void (*for_each)(struct dict_t *self, void *capture, void (*consumer)(void *capture, const void *key, const void *value));

    void *extra;
} dict_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool dict_delete(dict_t *dict);
void dict_clear(dict_t *dict);
bool dict_empty(const dict_t *dict);
bool dict_contains_key(const dict_t *dict, const void *key);
const struct vec_t *dict_keyset(const dict_t *dict);
const void *dict_get(const dict_t *dict, const void *key);
struct vec_t *dict_gets(const dict_t *dict, size_t num_keys, const void *keys);
bool dict_remove(dict_t *dict, size_t num_keys, const void *keys);
void dict_put(dict_t *dict, const void *key, const void *value);
void dict_puts(dict_t *dict, size_t num_elements, const void *keys, const void *values);
size_t dict_num_elements(dict_t *dict);
void dict_for_each(dict_t *dict, void *capture,
                   void (*consumer)(void *capture, const void *key, const void *value));