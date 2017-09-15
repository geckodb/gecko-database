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

#include <containers/dict.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool dict_delete(dict_t *dict)
{
    return DELEGATE_CALL(dict, free_dict);
}

void dict_clear(dict_t *dict)
{
    DELEGATE_CALL(dict, clear);
}

bool dict_empty(const dict_t *dict)
{
    return DELEGATE_CALL(dict, empty);
}

bool dict_contains_key(const dict_t *dict, const void *key)
{
    return DELEGATE_CALL_WARGS(dict, contains_key, key);
}

const struct vec_t *dict_keyset(const dict_t *dict)
{
    return DELEGATE_CALL(dict, keyset);
}

const void *dict_get(const dict_t *dict, const void *key)
{
    return DELEGATE_CALL_WARGS(dict, get, key);
}

struct vec_t *dict_gets(const dict_t *dict, size_t num_keys, const void *keys)
{
    return DELEGATE_CALL_WARGS(dict, gets, num_keys, keys);
}

bool dict_remove(dict_t *dict, size_t num_keys, const void *keys)
{
    return DELEGATE_CALL_WARGS(dict, remove, num_keys, keys);
}

void dict_put(dict_t *dict, const void *key, const void *value)
{
    DELEGATE_CALL_WARGS(dict, put, key, value);
}

void dict_puts(dict_t *dict, size_t num_elements, const void *keys, const void *values)
{
    DELEGATE_CALL_WARGS(dict, puts, num_elements, keys, values);
}

size_t dict_num_elements(dict_t *dict)
{
    return DELEGATE_CALL(dict, num_elements);
}

void dict_for_each(dict_t *dict, void *capture,
                   void (*consumer)(void *capture, const void *key, const void *value))
{
    DELEGATE_CALL_WARGS(dict, for_each, capture, consumer);
}
