// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_tuplet_field.h>
#include <gs_schema.h>
#include <gs_unsafe.h>

bool tuplet_field_next(tuplet_field_t *field, bool auto_next)
{
    assert (field);
    return field->_next(field, auto_next);
}

const void *tuplet_field_read(tuplet_field_t *field)
{
    assert (field);
    return field->_read(field);
}

void tuplet_field_update(tuplet_field_t *field, const void *data)
{
    assert (field);
    return field->_update(field, data);
}

bool tuplet_field_write(tuplet_field_t *field, const void *data, bool auto_next)
{
    tuplet_field_update(field, data);
    return tuplet_field_next(field, auto_next);
}

bool tuplet_field_write_eval(tuplet_field_t *field, bool eval, bool auto_next)
{
    return tuplet_field_write(field, &eval, auto_next);
}

void tuplet_field_set_null(tuplet_field_t *field)
{
    assert (field);
    return field->_set_null(field);
}

bool tuplet_field_is_null(tuplet_field_t *field)
{
    return field->_is_null(field);
}

void tuplet_field_open(tuplet_field_t *dst, tuplet_t *tuplet)
{
    tuplet_field_seek(dst, tuplet, 0);
}

void tuplet_field_seek(tuplet_field_t *dst, tuplet_t *tuplet, attr_id_t attr_id)
{
    assert(tuplet);
    tuplet->_open(dst, tuplet);
    dst->_seek(dst, attr_id);
}

size_t tuplet_field_size(tuplet_field_t *field)
{
    assert (field);
    panic_if((field->attr_id >= field->tuplet->fragment->schema->attr->num_elements), BADBOUNDS, "attribute tuplet_id invalid");
    const attr_t *attr = schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    return (attr_total_size(attr));
}

enum field_type tuplet_field_get_type(const tuplet_field_t *field)
{
    assert(field);
    return tuplet_field_type(field->tuplet, field->attr_id);
}

size_t tuplet_field_printlen(const tuplet_field_t *field)
{
    assert (field);
    assert (field->attr_value_ptr);
    enum field_type type = tuplet_field_get_type(field);
    const void *field_data = field->attr_value_ptr;
    return unsafe_field_println(type, field_data);
}


char *tuplet_field_str(const tuplet_field_t *field)
{
    assert (field);
    enum field_type type = tuplet_field_get_type(field);
    return unsafe_field_str(type, field->attr_value_ptr);
}