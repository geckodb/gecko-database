#include <field.h>

bool gs_field_next(field_t *field)
{
    assert (field);
    return field->_next(field);
}

const void *gs_field_read(field_t *field)
{
    assert (field);
    return field->_read(field);
}

void gs_field_update(field_t *field, const void *data)
{
    assert (field);
    return field->_update(field, data);
}

bool gs_field_write(field_t *field, const void *data)
{
    gs_field_update(field, data);
    return gs_field_next(field);
}

void gs_field_set_null(field_t *field)
{
    assert (field);
    return field->_set_null(field);
}

bool gs_field_is_null(field_t *field)
{
    return field->_is_null(field);
}

void gs_field_close(field_t *field)
{
    return field->_close(field);
}

field_t *gs_field_open(tuplet_t *tuplet)
{
    assert (tuplet);
    return tuplet->_open(tuplet);
}

size_t gs_field_size(field_t *field)
{
    assert (field);
    const attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    return (get_field_size_by_id(attr, field->attr_id));
}

size_t get_field_size_by_id(const attr_t *attr, attr_id_t attr_idx)
{
    return attr[attr_idx].type_rep * gs_sizeof(attr[attr_idx].type);
}