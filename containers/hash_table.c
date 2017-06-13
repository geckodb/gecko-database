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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <containers/hash_table.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

hash_table_t *hash_table_create();

bool hash_table_free(hash_table_t *table);

bool hash_table_clear(hash_table_t *table);

bool hash_table_is_empty(const hash_table_t *table);

bool hash_table_contains_values(const hash_table_t *table, size_t num_values, const void *values);

bool hash_table_contains_keys(const hash_table_t *table, size_t num_keys, const void *keys);

const void *hash_table_get(const hash_table_t *table, const void *key);

vector_t *hash_table_gets(const hash_table_t *table, size_t num_keys, const void *keys);

float hash_table_load_factor(hash_table_t *table);

bool hash_table_put(hash_table_t *table, const void *key, const void *value);

size_t hash_table_num_elements(hash_table_t *table);