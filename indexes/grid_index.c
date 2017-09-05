#include <indexes/grid_index.h>

grid_set_cursor_t *grid_set_cursor_create(size_t result_capacity)
{
    grid_set_cursor_t *result = require_good_malloc(sizeof(grid_set_cursor_t));
    *result = (grid_set_cursor_t) {
            .extra = vector_create(sizeof(struct grid_t *), result_capacity)
    };
    return result;
}

void grid_set_cursor_close(grid_set_cursor_t *cursor)
{
    vector_free(cursor->extra);
    free (cursor);
}

void grid_set_cursor_pushback(grid_set_cursor_t *cursor, const void *data)
{
    vector_add((cursor->extra), 1, data);
}

struct grid_t *grid_set_cursor_next(grid_set_cursor_t *result_set)
{
    static grid_set_cursor_t *dest;
    static size_t elem_idx;
    if (result_set != NULL) {
        dest = result_set;
        elem_idx = 0;
    }

    vector_t *vec = (vector_t * ) dest->extra;
    if (elem_idx < vec->num_elements) {
        return *(struct grid_t **) vector_at(vec, elem_idx++);
    } else return NULL;
}

size_t grid_set_cursor_numelem(const grid_set_cursor_t *result_set)
{
    require_non_null(result_set);
    return ((vector_t * ) result_set->extra)->num_elements;
}