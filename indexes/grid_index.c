#include <indexes/grid_index.h>

grid_index_result_cursor_t *grid_index_create_cursor(size_t result_capacity)
{
    grid_index_result_cursor_t *result = require_good_malloc(sizeof(grid_index_result_cursor_t));
    *result = (grid_index_result_cursor_t) {
            .extra = vector_create(sizeof(struct grid_t *), result_capacity)
    };
    return result;
}

void grid_index_close(grid_index_result_cursor_t *cursor)
{
    vector_free(cursor->extra);
    free (cursor);
}

const struct grid_t *grid_index_read(grid_index_result_cursor_t *result_set)
{
    static grid_index_result_cursor_t *dest;
    static size_t elem_idx;
    if (result_set != NULL) {
        dest = result_set;
        elem_idx = 0;
    }

    vector_t *vec = (vector_t * ) result_set->extra;
    if (elem_idx < vec->sizeof_element) {
        return (const struct grid_t *) vector_at(vec, elem_idx++);
    } else return NULL;
}