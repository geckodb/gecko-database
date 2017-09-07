#include <indexes/hindex.h>
#include <indexes/grid_index.h>
#include <schema.h>

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

grid_set_cursor_t *gs_hindex_query(const struct hindex_t *index, const tuple_id_t *tid_begin,
                                   const tuple_id_t *tid_end)
{
    REQUIRE_NONNULL(tid_begin);
    REQUIRE_NONNULL(tid_end);
    REQUIRE(tid_begin < tid_end, "Corrupted range");
    size_t approx_result_capacity = ((tid_end - tid_begin) * gs_schema_num_attributes(index->table_schema));
    grid_set_cursor_t *result = grid_set_cursor_create(approx_result_capacity);
    index->_query(result, index, tid_begin, tid_end);
    return result;
}

const struct grid_t *gs_hindex_query_read(grid_set_cursor_t *result_set)
{
    return grid_set_cursor_next(result_set);
}

void gs_hindex_query_close(grid_set_cursor_t *result_set)
{
    grid_set_cursor_close(result_set);
}