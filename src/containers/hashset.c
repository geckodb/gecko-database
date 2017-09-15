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

#include <containers/hashset.h>
#include <containers/dicts/hash_table.h>

void hashset_create(hashset_t *out, size_t elem_size, size_t capacity)
{
    REQUIRE_NONNULL(out)
    REQUIRE_NONZERO(elem_size)
    REQUIRE_NONZERO(capacity)
    out->dict = hash_table_new_jenkins(elem_size, sizeof(bool), capacity, 1.7f, 0.75f);
    out->vec  = vec_new(sizeof(elem_size), capacity);
}

void hashset_dispose(hashset_t *set)
{
    REQUIRE_NONNULL(set)
    dict_delete(set->dict);
    vec_free(set->vec);
    set->dict = NULL;
    set->vec = NULL;
}

void hashset_add(hashset_t *set, const void *data, size_t num_elems)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->vec)
    REQUIRE_NONNULL(set->dict)
    REQUIRE_NONNULL(data)
    REQUIRE_NONZERO(num_elems)
    if (!dict_contains_key(set->dict, data)) {
        vec_pushback(set->vec, num_elems, data);
        bool dummy;
        dict_put(set->dict, data, &dummy);
    }
}

void hashset_remove(hashset_t *set, const void *data, size_t num_elems)
{
    panic("Not implemented: '%s'. Hashtable remove is not implemented correctly currently", "hashset_remove");
}

bool hashset_contains(const hashset_t *set, const void *data)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->dict)
    return dict_contains_key(set->dict, data);
}

const void *hashset_begin(const hashset_t *set)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->vec)
    return vec_begin(set->vec);
}

const void *hashset_end(const hashset_t *set)
{
    REQUIRE_NONNULL(set)
    REQUIRE_NONNULL(set->vec)
    return vec_end(set->vec);
}