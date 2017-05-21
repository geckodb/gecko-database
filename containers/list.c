#include <containers/list.h>
#include <require.h>
#include <assert.h>

static bool _check_create_args(size_t element_size);
static const void *_get_data_ptr(const list_node_t *node);
static const list_node_t *_get_node_ptr(const void *data);

list_t *list_create(size_t element_size)
{
    list_t *list = NULL;
    if (_check_create_args(element_size) && ((list = require_malloc(sizeof(list_t))))) {
        list->element_size = element_size;
        list->num_elements = 0;
        list->root = list->tail = NULL;
    }
    return list;
}

bool list_free(list_t *list)
{
    bool freeable_list = list_clear(list);
    if (freeable_list) {
        free (list);
    }
    return freeable_list;
}

bool list_is_empty(const list_t *list)
{
    bool non_null = require_non_null(list);
    return (non_null && (list->num_elements == 0));
}

bool list_clear(list_t *list)
{
    bool non_null = require_non_null(list);
    if (non_null) {
        list_node_t *it = list->root, *next = NULL;
        while (it) {
            next = it->next;
            free(it->data);
            free(it);
            it = next;
        }
    }
    return non_null;
}

bool list_push(list_t *list, const void *data)
{
    list_node_t *node = NULL;
    if (require_non_null(list) && require_non_null(data) && ((node = require_malloc(sizeof(list_node_t)))) &&
        ((node->data = require_malloc(list->element_size)))) {
        node->prev = node->next = NULL;
        memcpy(node->data, data, list->element_size);
        if (list_is_empty(list)) {
            list->root = list->tail = node;
        } else {
            list->tail->next = node;
            node->prev = list->tail;
        }
        list->num_elements++;
    }
    return node;
}

const void *list_begin(const list_t *list)
{
    return (require_non_null(list) && (!list_is_empty(list))) ? _get_data_ptr(list->root) : NULL;
}

const void *list_next(const void *data)
{
    const list_node_t *node;
    return (require_non_null(data) && ((node = _get_node_ptr(data))) && (node->next)) ? _get_data_ptr(node) : NULL;
}

bool list_remove(list_t *list, const void *data)
{
    if (require_non_null(data)) {
        list_node_t *node = (list_node_t *) _get_node_ptr(data);
        if (node->prev != NULL) {
            node->prev = node->next;
        } else {
            list->root = node->next;
        }
        if (node->next != NULL) {
            ((list_node_t *) node->next)->prev = node->prev;
        } else {
            list->tail = node->prev;
        }
        free (node->data);
        free (node);
        return true;
    } else return false;
}

size_t list_num_elements(const list_t *list)
{
    return (require_non_null(list) ? list->element_size : 0);
}

bool _check_create_args(size_t element_size)
{
    return (element_size > 0);
}

const void *_get_data_ptr(const list_node_t *node)
{
    assert (node != NULL);
    return node + sizeof(list_node_t) - sizeof(void *);

}

const list_node_t *_get_node_ptr(const void *data)
{
    assert (data != NULL);
    return data - sizeof(list_node_t) + sizeof(void *);
}