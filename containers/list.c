// An implementation of the double-linked list data structure
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

#include <containers/list.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct _header {
    struct _header *prev, *next;
    list_t *list;
} header_t;

struct _list {
    header_t *root, *tail;
    size_t num_elements, element_size;
};

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static bool _check_create_args(size_t);
static void *_get_data_ptr(const header_t *);
static header_t *_get_node_ptr(const void *);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

list_t *list_create(size_t element_size)
{
    list_t *list = NULL;
    if (_check_create_args(element_size) && ((list = REQUIRE_MALLOC(sizeof(list_t))))) {
        list->element_size = element_size;
        list->num_elements = 0;
        list->root = list->tail = NULL;
    }
    return list;
}

void list_free(list_t *list)
{
    list_clear(list);
    free (list);
}

bool list_is_empty(const list_t *list)
{
    REQUIRE_NONNULL(list)
    return (list->num_elements == 0);
}

void list_clear(list_t *list)
{
    REQUIRE_NONNULL(list)
    header_t *it = list->root, *next = NULL;
    while (it) {
        next = it->next;
        free(it);
        it = next;
    }
}

bool list_push(list_t *list, const void *data)
{
    REQUIRE_NONNULL(list)
    REQUIRE_NONNULL(data)
    header_t *node = NULL;
    if ((node = REQUIRE_MALLOC(sizeof(header_t) + list->element_size))) {
        node->prev = node->next = NULL;
        memcpy(node + 1, data, list->element_size);
        if (list_is_empty(list)) {
            list->root = list->tail = node;
        } else {
            list->tail->next = node;
            node->prev = list->tail;
            list->tail = node;
        }
        node->list = list;
        list->num_elements++;
    }
    return node;
}

const void *list_begin(const list_t *list)
{
    REQUIRE_NONNULL(list)
    return ((!list_is_empty(list))) ? _get_data_ptr(list->root) : NULL;
}

const void *list_next(const void *data)
{
    REQUIRE_NONNULL(data)
    const header_t *node;
    return ((node = _get_node_ptr(data)) && (node->next) && (node->list->tail != node)) ?
            _get_data_ptr(node->next) : NULL;
}

void list_remove(const void *data)
{
    REQUIRE_NONNULL(data)
    header_t *node = _get_node_ptr(data);
    list_t *list = node->list;
    if (node->prev) {
        node->prev->next = node->next;
        if (node == list->tail)
            list->tail = node->prev;
    } else {
        if (node->next) {
            node->next->prev = NULL;
            list->root = node->next;
        } else
            list->root = list->tail = NULL;
    }
    list->num_elements--;
    free (node);

    assert ((list->root == NULL) || list->root->prev == NULL);
    assert ((list->tail == NULL) || list->tail->next == NULL);
    assert ((list->num_elements != 0) || ((list->tail == list->root) && (list->root == NULL)));
}

size_t list_num_elements(const list_t *list)
{
    REQUIRE_NONNULL(list)
    return list->num_elements;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool _check_create_args(size_t element_size)
{
    return (element_size > 0);
}

void *_get_data_ptr(const header_t *node)
{
    assert (node != NULL);
    return (void *) (node + 1);

}

header_t *_get_node_ptr(const void *data)
{
    assert (data != NULL);
    return (void *)(data - sizeof(header_t));
}