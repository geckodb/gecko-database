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

#include <containers/gs_freelist.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_freelist_create(gs_freelist_t *list, size_t elem_size, size_t capacity, gs_init_t init, gs_inc_t inc)
{
    GS_REQUIRE_NONNULL(list);
    GS_REQUIRE_NONNULL(inc);
    REQUIRE((elem_size > 0), BADINT);
    list->free_elem = gs_vec_new(elem_size, capacity);
    list->next_element = GS_REQUIRE_MALLOC(elem_size);
    list->inc = inc;
    init(list->next_element);
}

void gs_freelist_create2(gs_freelist_t **list, size_t elem_size, size_t capacity, gs_init_t init, gs_inc_t inc)
{
    gs_freelist_t *result = GS_REQUIRE_MALLOC(sizeof(gs_freelist_t));
    gs_freelist_create(result, elem_size, capacity, init, inc);
    *list = result;
}

void gs_freelist_dispose(gs_freelist_t *list)
{
    GS_REQUIRE_NONNULL(list);
    gs_vec_free(list->free_elem);
    free(list->next_element);
}

void gs_freelist_free(gs_freelist_t *list)
{
    gs_freelist_dispose(list);
    free(list);
}

void gs_freelist_bind(void *out, const gs_freelist_t *list, size_t num_elem)
{
    GS_REQUIRE_NONNULL(out);
    GS_REQUIRE_NONNULL(list);
    REQUIRE((num_elem > 0), BADINT);
    while (num_elem--) {
        size_t sizeof_element = list->free_elem->sizeof_element;
        const void *data;

        if (list->free_elem->num_elements > 0) {
            data = gs_vec_pop_unsafe(list->free_elem);
            memcpy(out, data, sizeof_element);
        } else {
            data = list->next_element;
            memcpy(out, data, sizeof_element);
            list->inc(list->next_element);
        }

        out += sizeof_element;
    }
}

const void *gs_freelist_peek_new(const gs_freelist_t *list)
{
    GS_REQUIRE_NONNULL(list);
    return list->next_element;
}

void gs_freelist_pushback(gs_freelist_t *list, size_t num_elem, void *elem)
{
    GS_REQUIRE_NONNULL(list);
    GS_REQUIRE_NONNULL(elem);
    REQUIRE((num_elem > 0), BADINT);
    gs_vec_pushback(list->free_elem, num_elem, elem);
}