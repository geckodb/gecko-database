#include <tuple_cursor.h>

void gs_tuple_cursor_create(tuple_cursor_t *resultset, struct grid_table_t *context, tuple_id_t *tuple_ids,
                            size_t ntuple_ids)
{
    REQUIRE_NONNULL(resultset);
    resultset->context = context;
    resultset->ntuple_ids = ntuple_ids;
    resultset->tuple_ids = tuple_ids;
    gs_tuple_cursor_rewind(resultset);
}

void gs_tuple_cursor_free(tuple_cursor_t *resultset)
{
    REQUIRE_NONNULL(resultset);
    free (resultset->tuple_ids);
}

void gs_tuple_cursor_rewind(tuple_cursor_t *resultset)
{
    REQUIRE_NONNULL(resultset);
    resultset->tuple_id_cursor = 0;
}

bool gs_tuple_cursor_next(tuple_t *tuple, tuple_cursor_t *resultset)
{
    REQUIRE_NONNULL(resultset);
    REQUIRE_NONNULL(tuple);
    if (resultset->tuple_id_cursor < resultset->ntuple_ids) {
        gs_tuple_open(tuple, resultset->context, resultset->tuple_ids[resultset->tuple_id_cursor++]);
        return true;
    } else return false;
}
