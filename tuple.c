#include <tuple.h>

tuple_t *gs_tuple_open(struct grid_table *table, tuple_id_t tuple_id)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuple_open))
    return NULL;
}

void gs_tuple_close(tuple_t *tuple)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuple_close))
}

bool gs_tuple_next(tuple_t *tuple)
{
    panic(NOTIMPLEMENTED, to_string(tuple))
    return false;
}

tuple_t *gs_tuple_rewind(tuple_t *tuple)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuplet_rewind))
    return NULL;
}

void gs_tuple_set_null(tuple_t *tuple)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuple_set_null))
}

bool gs_tuple_is_null(tuple_t *tuple)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuple_is_null))
    return false;
}

size_t gs_tuple_size(tuple_t *tuple)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuple_size))
    return 0;
}