#include <resultset.h>

void gs_resultset_create(resultset_t *resultset, struct grid_table_t *context, tuple_id_t *tuple_ids, size_t ntuple_ids)
{
    REQUIRE_NONNULL(resultset);
    resultset->context = context;
    resultset->ntuple_ids = ntuple_ids;
    resultset->tuple_ids = tuple_ids;
    gs_resultset_rewind(resultset);
}

void gs_resultset_free(resultset_t *resultset)
{
    REQUIRE_NONNULL(resultset);
    free (resultset->tuple_ids);
}

void gs_resultset_rewind(resultset_t *resultset)
{
    REQUIRE_NONNULL(resultset);
    resultset->tuple_id_cursor = 0;
}

bool gs_resultset_next(tuple_t *tuple, resultset_t *resultset)
{
    REQUIRE_NONNULL(resultset);
    REQUIRE_NONNULL(tuple);
    if (resultset->tuple_id_cursor < resultset->ntuple_ids) {
        gs_tuple_open(tuple, resultset->context, resultset->tuple_ids[resultset->tuple_id_cursor++]);
        return true;
    } else return false;
}
