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

#include <containers/dictionaries/hash_table.h>
#include <error.h>
#include <msg.h>
#include <macros.h>
#include <require.h>
#include <object.h>
#include <containers/dictionary.h>

// ---------------------------------------------------------------------------------------------------------------------
// H A S H   C O D E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t hash_code_size_t(const void *data)
{
    return *((size_t *)data);
}

// ---------------------------------------------------------------------------------------------------------------------
// H A S H   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t hash_fn_mod(size_t code, size_t upper_bound)
{
    return (code % upper_bound);
}

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void hash_table_to_string(void *self, FILE *out);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void hash_table_create(hash_table_t *table, hash_code_fn_t hash_code_fn, hash_fn_t hash_fn, size_t key_size,
                       size_t elem_size, dictionary_clear_fn_t clear, dictionary_empty_fn_t empty,
                       dictionary_contains_values_fn_t contains_values, dictionary_contains_keys_fn_t contains_keys,
                       dictionary_get_fn_t get, dictionary_gets_fn_t gets, dictionary_remove_fn_t remove,
                       dictionary_put_fn_t put, dictionary_num_elements_fn_t num_elements)
{
    require_nonnull(table);
    require_nonnull(hash_code_fn);
    require_nonnull(hash_fn);

    *table = (hash_table_t) {
        .protected = {
            .methods = {
                .hash_code_fn = hash_code_fn,
                .hash_fn = hash_fn
            }
        }
    };

    dictionary_create(&table->protected.members.base, key_size, elem_size, clear, empty, contains_values,
                      contains_keys, get, gets, remove, put, num_elements);

    hash_table_override_to_string(table, hash_table_to_string);
}

void hash_table_override_to_string(hash_table_t *table, object_to_string_fn_t f)
{
    require_nonnull(table);
    require_nonnull(f);
    dictionary_override_to_string(&table->protected.members.base, f);
}

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void hash_table_to_string(void *self, FILE *out)
{
    require_nonnull(self);
    require_nonnull(out);
    fprintf(out, "hash_table(adr=%p)", self);
}