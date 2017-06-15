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


void dict_clear(dict_t *dict)
{
    delegte_call(dict, clear);
}

bool dict_empty(const dict_t *dict)
{
    return delegte_call(dict, empty);
}

bool dict_contains_values(const dict_t *dict, size_t num_values, const void *values)
{
    return delegte_call_wargs(dict, contains_values, num_values, values);
}

bool dict_contains_keys(const dict_t *dict, size_t num_keys, const void *keys)
{
    return delegte_call_wargs(dict, contains_keys, num_keys, keys);
}

const void *dict_get(const dict_t *dict, const void *key)
{
    return delegte_call_wargs(dict, get, key);
}

vector_t *dict_gets(const dict_t *dict, size_t num_keys, const void *keys)
{
    return delegte_call_wargs(dict, gets, num_keys, keys);
}

bool dict_remove(dict_t *dict, size_t num_keys, const void *keys)
{
    return delegte_call_wargs(dict, remove, num_keys, keys);
}

void dict_put(dict_t *dict, const void *key, const void *value)
{
    delegte_call_wargs(dict, put, key, value);
}

void dict_puts(dict_t *dict, size_t num_elements, const void *keys, const void *values)
{
    delegte_call_wargs(dict, puts, num_elements, keys, values);
}

size_t dict_num_elements(dict_t *dict)
{
    return delegte_call(dict, num_elements);
}

void dict_for_each(dict_t *dict, void *capture,
                   void (*consumer)(void *capture, const void *key, const void *value))
{
    delegte_call_wargs(dict, for_each, capture, consumer);
}
