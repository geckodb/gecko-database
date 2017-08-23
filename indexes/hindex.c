#include <indexes/hindex.h>

void gs_hindex_free(struct hindex_t *index)
{
    delegte_call(index, _free);
}

void gs_hindex_add(struct hindex_t *index, const tuple_id_interval_t *key, const struct grid_t *grid)
{
    delegte_call_wargs(index, _add, key, grid);
}

void gs_hindex_remove_interval(struct hindex_t *index, const tuple_id_interval_t *key)
{
    delegte_call_wargs(index, _remove_interval, key);
}

void gs_hindex_remove_intersec(struct hindex_t *index, tuple_id_t tid)
{
    delegte_call_wargs(index, _remove_intersec, tid);
}

bool gs_hindex_contains(const struct hindex_t *index, tuple_id_t tid)
{
    return delegte_call_wargs(index, _contains, tid);
}

hindex_result_t *gs_hindex_query_open(const struct hindex_t *index, const tuple_id_t *tid_begin,
                                      const tuple_id_t *tid_end)
{
    return delegte_call_wargs(index, _query_open, tid_begin, tid_end);
}

const struct grid_t *gs_hindex_query_read(const struct hindex_t *index, hindex_result_t *result_set)
{
    return delegte_call_wargs(index, _query_read, result_set);
}

void gs_hindex_query_close(const struct hindex_t *index, hindex_result_t *result_set)
{
    delegte_call_wargs2(index, _query_close, result_set);
}