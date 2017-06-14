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
#include <require.h>
#include <msg.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define delegte_call(dic, fun)                                                                                         \
    ({                                                                                                                 \
        require_nonnull(dic);                                                                                          \
        require_impl(dic->fun);                                                                                        \
        dic->fun(dic);                                                                                                 \
    })

#define delegte_call_wargs(dic, fun, ...)                                                                              \
    ({                                                                                                                 \
        require_nonnull(dic);                                                                                          \
        require_impl(dic->fun);                                                                                        \
        dic->fun(dic,__VA_ARGS__);                                                                                     \
    })


// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------


void dictionary_clear(dictionary_t *dictionary)
{
    delegte_call(dictionary, clear);
}

bool dictionary_empty(const dictionary_t *dictionary)
{
    return delegte_call(dictionary, empty);
}

bool dictionary_contains_values(const dictionary_t *dictionary, size_t num_values, const void *values)
{
    return delegte_call_wargs(dictionary, contains_values, num_values, values);
}

bool dictionary_contains_keys(const dictionary_t *dictionary, size_t num_keys, const void *keys)
{
    return delegte_call_wargs(dictionary, contains_keys, num_keys, keys);
}

const void *dictionary_get(const dictionary_t *dictionary, const void *key)
{
    return delegte_call_wargs(dictionary, get, key);
}

vector_t *dictionary_gets(const dictionary_t *dictionary, size_t num_keys, const void *keys)
{
    return delegte_call_wargs(dictionary, gets, num_keys, keys);
}

bool dictionary_remove(dictionary_t *dictionary, size_t num_keys, const void *keys)
{
    return delegte_call_wargs(dictionary, remove, num_keys, keys);
}

void dictionary_put(dictionary_t *dictionary, const void *key, const void *value)
{
    delegte_call_wargs(dictionary, put, key, value);
}

void dictionary_puts(dictionary_t *dictionary, size_t num_elements, const void *keys, const void *values)
{
    delegte_call_wargs(dictionary, puts, num_elements, keys, values);
}

size_t dictionary_num_elements(dictionary_t *dictionary)
{
    return delegte_call(dictionary, num_elements);
}

void dictionary_for_each(dictionary_t *dictionary, void *capture, void (*consumer)(void *capture, const void *key, const void *value))
{
    delegte_call_wargs(dictionary, for_each, capture, consumer);
}
