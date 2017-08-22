#include <indexes/grid_index.h>

void grid_index_free(grid_index_t *index)
{
    delegte_call(index, _free);
}

void grid_index_add(grid_index_t *index, size_t key, const struct grid_t *grid)
{
    delegte_call_wargs(index, _add, key, grid);
}

void grid_index_remove(grid_index_t *index, size_t key)
{
    delegte_call_wargs(index, _remove, key);
}

bool grid_index_contains(const grid_index_t *index, size_t key)
{
    return delegte_call_wargs(index, _contains, key);
}

index_query_t *grid_index_query_open(const struct grid_index_t *index, size_t key_range_begin, size_t key_range_end)
{
    return delegte_call_wargs(index, _query_open, key_range_begin, key_range_end);
}

index_query_t *grid_index_query_append(const struct grid_index_t *index, index_query_t *result, size_t key_range_begin, size_t key_range_end)
{
    return delegte_call_wargs(index, _query_append, result, key_range_begin, key_range_end);
}

const struct grid_t *grid_index_query_read(const struct grid_index_t *index, index_query_t *result_set)
{
    return delegte_call_wargs(index, _query_read, result_set);
}

void grid_index_query_close(const struct grid_index_t *index, index_query_t *result_set)
{
    delegte_call_wargs2(index, _query_close, result_set);
}
