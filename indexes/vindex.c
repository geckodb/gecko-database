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

vindex_result_t *gs_vindex_query_open(const struct vindex_t *index, const void *key_range_begin,
                                      const void *key_range_end)
{
    return delegte_call_wargs(index, _query_open, key_range_begin, key_range_end);
}

vindex_result_t *gs_vindex_query_append(const struct vindex_t *index, vindex_result_t *result,
                                        const void *key_range_begin, const void *key_range_end)
{
    return delegte_call_wargs(index, _query_append, result, key_range_begin, key_range_end);
}

const struct grid_t *gs_vindex_query_read(const struct vindex_t *index, vindex_result_t *result_set)
{
    return delegte_call_wargs(index, _query_read, result_set);
}

void gs_vindex_query_close(const struct vindex_t *index, vindex_result_t *result_set)
{
    delegte_call_wargs2(index, _query_close, result_set);
}
