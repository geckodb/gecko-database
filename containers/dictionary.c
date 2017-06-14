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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <containers/dictionary.h>
#include <error.h>
#include <msg.h>
#include <macros.h>
#include <require.h>
#include <object.h>

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void dictionary_to_string(void *self, FILE *out);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void dictionary_create(dictionary_t *dictionary, size_t key_size, size_t elem_size,
                       dictionary_clear_fn_t clear, dictionary_empty_fn_t empty,
                       dictionary_contains_values_fn_t contains_values, dictionary_contains_keys_fn_t contains_keys,
                       dictionary_get_fn_t get, dictionary_gets_fn_t gets, dictionary_remove_fn_t remove,
                       dictionary_put_fn_t put, dictionary_num_elements_fn_t num_elements)
{
    require_nonnull(dictionary);
    require_not_zero(key_size);
    require_not_zero(elem_size);
    *dictionary = (dictionary_t) {
        .protected = {
            .members = {
                .elem_size = elem_size,
                .key_size = key_size
            }
        },
        .public = {
            .methods = {
                .clear = clear,
                .is_empty = empty,
                .contains_values = contains_values,
                .contains_keys = contains_keys,
                .get = get,
                .gets = gets,
                .remove = remove,
                .put = put,
                .num_elements = num_elements
            }
        }
    };
    object_create(&dictionary->protected.members.base);
    dictionary_override_to_string(dictionary, dictionary_to_string);
}

void dictionary_override_to_string(dictionary_t *dictionary, object_to_string_fn_t f)
{
    require_nonnull(dictionary);
    require_nonnull(f);
    object_override(&dictionary->protected.members.base, f);
}

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void dictionary_to_string(void *self, FILE *out)
{
    require_nonnull(self);
    require_nonnull(out);
    fprintf(out, "dictionary(adr=%p)", self);
}
