#include <tuple_field.h>

tuple_field_t *gs_tuple_field_open(tuple_t *tuple)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuple_field_open))
    return NULL;
}

bool gs_tuple_field_next(tuple_field_t *field)
{
    panic(NOTIMPLEMENTED, to_string(gs_field_next))
    return false;
}

const void *gs_tuple_field_read(tuple_field_t *field)
{
    panic(NOTIMPLEMENTED, to_string(gs_field_read))
    return NULL;
}

void gs_tuple_field_update(tuple_field_t *field, const void *data)
{
    panic(NOTIMPLEMENTED, to_string(gs_field_update))
}

bool gs_tuple_field_write(tuple_field_t *field, const void *data)
{
    panic(NOTIMPLEMENTED, to_string(gs_field_write))
    return false;
}

void gs_tuple_field_set_null(tuple_field_t *field)
{
    panic(NOTIMPLEMENTED, to_string(gs_field_is_null))
}

bool gs_tuple_field_is_null(tuple_field_t *field)
{
    panic(NOTIMPLEMENTED, to_string(gs_field_is_null))
    return false;
}

void gs_tuple_field_close(tuple_field_t *field)
{
    panic(NOTIMPLEMENTED, to_string(gs_field_close))
}