#include <indexes/vindex.h>

void gs_vindex_free(vindex_t *index)
{
    delegte_call(index, _free);
}

void gs_vindex_add(vindex_t *index, const attr_id_t *key, const struct grid_t *grid)
{
    delegte_call_wargs(index, _add, key, grid);
}

void gs_vindex_remove(vindex_t *index, const attr_id_t *key)
{
    delegte_call_wargs(index, _remove, key);
}

bool gs_vindex_contains(const vindex_t *index, const attr_id_t *key)
{
    return delegte_call_wargs(index, _contains, key);
}

grid_cursor_t *gs_vindex_query(const struct vindex_t *index, const attr_id_t *key_range_begin,
                                   const attr_id_t *key_range_end)
{
    REQUIRE_NONNULL(key_range_begin);
    REQUIRE_NONNULL(key_range_end);
    REQUIRE(key_range_begin < key_range_end, "Corrupted range");
    size_t result_capacity = (key_range_end - key_range_begin);
    grid_cursor_t *result = grid_cursor_create(result_capacity);
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

grid_cursor_t *gs_vindex_query_append(const struct vindex_t *index, grid_cursor_t *result,
                                        const attr_id_t *key_range_begin, const attr_id_t *key_range_end)
{
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

const struct grid_t *gs_vindex_query_read(grid_cursor_t *result_set)
{
    return grid_cursor_next(result_set);
}

void gs_vindex_query_close(grid_cursor_t *result_set)
{
    grid_cursor_close(result_set);
}
