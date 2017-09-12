#include <grid_cursor.h>
#include <containers/dicts/hash_table.h>

grid_cursor_t *grid_cursor_create(size_t cursor)
{
    grid_cursor_t *result = REQUIRE_MALLOC(sizeof(grid_cursor_t));
    *result = (grid_cursor_t) {
            .extra = vector_create(sizeof(struct grid_t *), cursor)
    };
    return result;
}

void grid_cursor_close(grid_cursor_t *cursor)
{
    vector_free(cursor->extra);
    free (cursor);
}

void grid_cursor_pushback(grid_cursor_t *cursor, const void *data)
{
    vector_add((cursor->extra), 1, data);
}

void grid_cursor_append(grid_cursor_t *dst, grid_cursor_t *src)
{
    vector_add_all(dst->extra, src->extra);
}

void grid_cursor_dedup(grid_cursor_t *cursor)
{
    vector_dedup(cursor->extra);
}

struct grid_t *grid_cursor_next(grid_cursor_t *cursor)
{
    static grid_cursor_t *dest;
    static size_t elem_idx;
    if (cursor != NULL) {
        dest = cursor;
        elem_idx = 0;
    }

    vector_t *vec = (vector_t * ) dest->extra;
    if (elem_idx < vec->num_elements) {
        return *(struct grid_t **) vector_at(vec, elem_idx++);
    } else return NULL;
}

size_t grid_cursor_numelem(const grid_cursor_t *cursor)
{
    REQUIRE_NONNULL(cursor);
    return ((vector_t * ) cursor->extra)->num_elements;
}

bool gs_grid_cursor_is_empty(const grid_cursor_t *cursor)
{
    REQUIRE_NONNULL(cursor)
    return (vector_num_elements(cursor->extra) == 0);
}