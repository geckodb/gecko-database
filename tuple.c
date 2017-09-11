#include <tuple.h>
#include <grid.h>

void gs_tuple_open(tuple_t *tuple, const struct grid_table_t *table, tuple_id_t tuple_id)
{
    REQUIRE_NONNULL(tuple);
    REQUIRE_NONNULL(table);
    tuple->table = table;
    tuple->tuple_id = tuple_id;
}

void gs_tuple_id_init(void *data)
{
    *((tuple_id_t *) data) = 0;
}

void gs_tuple_id_inc(void *data)
{
    *((tuple_id_t *) data) += 1;
}
