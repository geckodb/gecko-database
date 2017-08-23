#include <containers/freelist.h>

void gs_freelist_create(freelist_t *list, size_t elem_size, size_t capacity, init_t init, inc_t inc)
{
    require_non_null(list);
    require_non_null(inc);
    require((elem_size > 0), BADINT);
    list->free_elem = vector_create(elem_size, capacity);
    list->next_element = require_good_malloc(elem_size);
    list->inc = inc;
    init(list->next_element);
}

void gs_freelist_free(freelist_t *list)
{
    require_non_null(list);
    vector_free(list->free_elem);
    free(list->next_element);
}

void gs_freelist_bind(void *out, const freelist_t *list, size_t num_elem)
{
    require_non_null(out);
    require_non_null(list);
    require((num_elem > 0), BADINT);
    while (num_elem--) {
        size_t sizeof_element = list->free_elem->sizeof_element;
        const void *data;

        if (list->free_elem->num_elements > 0) {
            data = vector_pop_unsafe(list->free_elem);
            memcpy(out, data, sizeof_element);
        } else {
            data = list->next_element;
            memcpy(out, data, sizeof_element);
            list->inc(list->next_element);
        }


        out += sizeof_element;
    }
}

const void *gs_freelist_peek_new(const freelist_t *list)
{
    require_non_null(list);
    return list->next_element;
}

void gs_freelist_pushback(freelist_t *list, size_t num_elem, void *elem)
{
    require_non_null(list);
    require_non_null(elem);
    require((num_elem > 0), BADINT);
    vector_add(list->free_elem, num_elem, elem);
}