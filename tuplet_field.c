#include <tuplet_field.h>
#include <schema.h>
#include <unsafe.h>

bool gs_tuplet_field_next(tuplet_field_t *field)
{
    assert (field);
    return field->_next(field);
}

const void *gs_tuplet_field_read(tuplet_field_t *field)
{
    assert (field);
    return field->_read(field);
}

void gs_tuplet_field_update(tuplet_field_t *field, const void *data)
{
    assert (field);
    return field->_update(field, data);
}

bool gs_tuplet_field_write(tuplet_field_t *field, const void *data)
{
    gs_tuplet_field_update(field, data);
    return gs_tuplet_field_next(field);
}

bool gs_tuplet_field_write_eval(tuplet_field_t *field, bool eval)
{
    return gs_tuplet_field_write(field, &eval);
}

void gs_tuplet_field_set_null(tuplet_field_t *field)
{
    assert (field);
    return field->_set_null(field);
}

bool gs_tuplet_field_is_null(tuplet_field_t *field)
{
    return field->_is_null(field);
}

void gs_tuplet_field_close(tuplet_field_t *field)
{
    return field->_close(field);
}

tuplet_field_t *gs_tuplet_field_open(tuplet_t *tuplet)
{
    return gs_tuplet_field_seek(tuplet, 0);
}

tuplet_field_t *gs_tuplet_field_seek(tuplet_t *tuplet, attr_id_t attr_id)
{
    assert(tuplet);
    tuplet_field_t *field = tuplet->_open(tuplet);
    field->_seek(field, attr_id);
    return field;
}

size_t gs_tuplet_field_size(tuplet_field_t *field)
{
    assert (field);
    panic_if((field->attr_id >= field->tuplet->fragment->schema->attr->num_elements), BADBOUNDS, "attribute tuplet_id invalid");
    const attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    return (gs_attr_total_size(attr));
}

size_t gs_attr_total_size(const struct attr_t *attr)
{
    return attr->type_rep * gs_field_type_sizeof(attr->type);
}

enum field_type gs_tuplet_field_get_type(const tuplet_field_t *field)
{
    assert(field);
    return gs_tuplet_get_field_type(field->tuplet, field->attr_id);
}

size_t gs_tuplet_field_get_printlen(const tuplet_field_t *field)
{
    assert (field);
    assert (field->attr_value_ptr);
    enum field_type type = gs_tuplet_field_get_type(field);
    const void *field_data = field->attr_value_ptr;
    return gs_unsafe_field_get_println(type, field_data);
}


char *gs_tuplet_field_to_string(const tuplet_field_t *field)
{
    assert (field);
    enum field_type type = gs_tuplet_field_get_type(field);
    return gs_unsafe_field_to_string(type, field->attr_value_ptr);
}