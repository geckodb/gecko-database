// An implementation of the vector data structure
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
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

#include <containers/freelist.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void freelist_create(freelist_t *list, size_t elem_size, size_t capacity, init_t init, inc_t inc)
{
    GS_REQUIRE_NONNULL(list);
    GS_REQUIRE_NONNULL(inc);
    REQUIRE((elem_size > 0), BADINT);
    list->free_elem = vec_new(elem_size, capacity);
    list->next_element = GS_REQUIRE_MALLOC(elem_size);
    list->inc = inc;
    init(list->next_element);
}

void freelist_create2(freelist_t **list, size_t elem_size, size_t capacity, init_t init, inc_t inc)
{
    freelist_t *result = GS_REQUIRE_MALLOC(sizeof(freelist_t));
    freelist_create(result, elem_size, capacity, init, inc);
    *list = result;
}

void freelist_dispose(freelist_t *list)
{
    GS_REQUIRE_NONNULL(list);
    vec_free(list->free_elem);
    free(list->next_element);
}

void freelist_free(freelist_t *list)
{
    freelist_dispose(list);
    free(list);
}

void freelist_bind(void *out, const freelist_t *list, size_t num_elem)
{
    GS_REQUIRE_NONNULL(out);
    GS_REQUIRE_NONNULL(list);
    REQUIRE((num_elem > 0), BADINT);
    while (num_elem--) {
        size_t sizeof_element = list->free_elem->sizeof_element;
        const void *data;

        if (list->free_elem->num_elements > 0) {
            data = vec_pop_unsafe(list->free_elem);
            memcpy(out, data, sizeof_element);
        } else {
            data = list->next_element;
            memcpy(out, data, sizeof_element);
            list->inc(list->next_element);
        }

        out += sizeof_element;
    }
}

const void *freelist_peek_new(const freelist_t *list)
{
    GS_REQUIRE_NONNULL(list);
    return list->next_element;
}

void freelist_pushback(freelist_t *list, size_t num_elem, void *elem)
{
    GS_REQUIRE_NONNULL(list);
    GS_REQUIRE_NONNULL(elem);
    REQUIRE((num_elem > 0), BADINT);
    vec_pushback(list->free_elem, num_elem, elem);
}