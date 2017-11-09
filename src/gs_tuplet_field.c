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

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool gs_tuplet_field_next(gs_tuplet_field_t *field, bool auto_next)
{
    assert (field);
    return field->_next(field, auto_next);
}

const void *gs_tuplet_field_read(gs_tuplet_field_t *field)
{
    assert (field);
    return field->_read(field);
}

void gs_tuplet_field_update(gs_tuplet_field_t *field, const void *data)
{
    assert (field);
    return field->_update(field, data);
}

bool gs_tuplet_field_write(gs_tuplet_field_t *field, const void *data, bool auto_next)
{
    gs_tuplet_field_update(field, data);
    return gs_tuplet_field_next(field, auto_next);
}

bool gs_tuplet_field_write_eval(gs_tuplet_field_t *field, bool eval, bool auto_next)
{
    return gs_tuplet_field_write(field, &eval, auto_next);
}

void gs_tuplet_field_set_null(gs_tuplet_field_t *field)
{
    assert (field);
    return field->_set_null(field);
}

bool gs_tuplet_field_is_null(gs_tuplet_field_t *field)
{
    return field->_is_null(field);
}

void gs_tuplet_field_open(gs_tuplet_field_t *dst, gs_tuplet_t *tuplet)
{
    gs_tuplet_field_seek(dst, tuplet, 0);
}

void gs_tuplet_field_seek(gs_tuplet_field_t *dst, gs_tuplet_t *tuplet, gs_attr_id_t attr_id)
{
    assert(tuplet);
    tuplet->_open(dst, tuplet);
    dst->_seek(dst, attr_id);
}

size_t gs_tuplet_field_size(gs_tuplet_field_t *field)
{
    assert (field);
    panic_if((field->attr_id >= field->tuplet->fragment->schema->attr->num_elements), BADBOUNDS, "attribute tuplet_id invalid");
    const gs_attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    return (gs_attr_total_size(attr));
}

enum gs_field_type_e gs_tuplet_field_get_type(const gs_tuplet_field_t *field)
{
    assert(field);
    return gs_tuplet_field_type(field->tuplet, field->attr_id);
}

size_t gs_tuplet_field_printlen(const gs_tuplet_field_t *field)
{
    assert (field);
    assert (field->attr_value_ptr);
    enum gs_field_type_e type = gs_tuplet_field_get_type(field);
    const void *field_data = field->attr_value_ptr;
    return gs_unsafe_field_println(type, field_data);
}


char *gs_tuplet_field_str(const gs_tuplet_field_t *field)
{
    assert (field);
    enum gs_field_type_e type = gs_tuplet_field_get_type(field);
    return gs_unsafe_field_str(type, field->attr_value_ptr);
}