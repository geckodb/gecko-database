#include <indexes/vindex.h>

void gs_vindex_free(vindex_t *index)
{
    delegte_call(index, _free);
}

void gs_vindex_add(vindex_t *index, const void *key, const struct grid_t *grid)
{
    delegte_call_wargs(index, _add, key, grid);
}

void gs_vindex_remove(vindex_t *index, const void *key)
{
    delegte_call_wargs(index, _remove, key);
}

bool gs_vindex_contains(const vindex_t *index, const void *key)
{
    return delegte_call_wargs(index, _contains, key);
}

grid_index_result_cursor_t *gs_vindex_query_open(const struct vindex_t *index, const void *key_range_begin,
                                      const void *key_range_end)
{
    require_non_null(key_range_begin);
    require_non_null(key_range_end);
    require(key_range_begin < key_range_end, "Corrupted range");
    size_t result_capacity = (key_range_end - key_range_begin);
    grid_index_result_cursor_t *result = grid_index_create_cursor(result_capacity);
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

grid_index_result_cursor_t *gs_vindex_query_append(const struct vindex_t *index, grid_index_result_cursor_t *result,
                                        const void *key_range_begin, const void *key_range_end)
{
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

const struct grid_t *gs_vindex_query_read(grid_index_result_cursor_t *result_set)
{
    return grid_index_read(result_set);
}

void gs_vindex_query_close(grid_index_result_cursor_t *result_set)
{
    grid_index_close(result_set);
}
