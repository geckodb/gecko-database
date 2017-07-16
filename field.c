#include <field.h>
#include <schema.h>
#include <unsafe.h>

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
    panic_if((field->attr_id >= field->tuplet->fragment->schema->attr->num_elements), BADBOUNDS, "attribute tuplet_id invalid");
    const attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    return (gs_attr_total_size(attr));
}

size_t gs_attr_total_size(const attr_t *attr)
{
    return attr->type_rep * gs_field_type_sizeof(attr->type);
}

enum field_type gs_field_get_type(const field_t *field)
{
    assert(field);
    return gs_tuplet_get_field_type(field->tuplet, field->attr_id);
}

size_t gs_field_get_printlen(const field_t *field)
{
    assert (field);
    assert (field->attr_value_ptr);
    enum field_type type = gs_field_get_type(field);
    const void *field_data = field->attr_value_ptr;
    return gs_unsafe_field_get_println(type, field_data);
}


char *gs_field_to_string(const field_t *field)
{
    assert (field);
    enum field_type type = gs_field_get_type(field);
    return gs_unsafe_field_to_string(type, field->attr_value_ptr);
}