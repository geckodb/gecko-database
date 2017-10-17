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

#include <gs_attr.h>
#include <containers/gs_vec.h>
#include <gs_tuplet_field.h>
#include <gs_frag.h>
#include <gs_schema.h>

#define DEFINE_ATTRIBUTE_CREATE(type_name,internal_type)                                                               \
attr_id_t attr_create_##type_name(const char *name, schema_t *schema) {                                                \
    return attr_create(name, internal_type, 1, schema);                                                                \
}

#define DEFINE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                         \
attr_id_t attr_create_##type_name(const char *name, size_t length, schema_t *schema) {                                 \
    return attr_create(name, internal_type, length, schema);                                                           \
}

attr_id_t _attr_create(const char *name, enum field_type data_type, size_t data_type_rep, ATTR_FLAGS attr_flags, schema_t *schema)
{
    panic_if((name == NULL || schema == NULL || strlen(name) + 1 > ATTR_NAME_MAXLEN), BADARG,
             "null pointer or attribute name limit exceeded");

    attr_t attr = {
            .id     = schema->attr->num_elements,
            .type     = data_type,
            .type_rep = data_type_rep,
            .flags    = attr_flags,
            .str_format_mlen   = 0,
    };

    strcpy(attr.name, name);
    memset(attr.checksum, 0, MD5_DIGEST_LENGTH);
    vec_pushback(schema->attr, 1, &attr);
    return attr.id;
}

const char *attr_name(const struct attr_t *attr)
{
    assert (attr);
    return attr->name;
}

size_t attr_str_max_len(attr_t *attr)
{
    assert (attr);
    return attr->str_format_mlen;
}

enum field_type attr_type(const attr_t *attr)
{
    assert (attr);
    return attr->type;
}

const attr_t *attr_cpy(const attr_t *template, schema_t *new_owner)
{
    assert (template);
    assert(new_owner);
    attr_id_t id = _attr_create(template->name, template->type, template->type_rep, template->flags, new_owner);
    return schema_attr_by_id(new_owner, id);
}

size_t attr_total_size(const struct attr_t *attr)
{
    return attr->type_rep * field_type_sizeof(attr->type);
}

attr_id_t attr_create(const char *name, enum field_type data_type, size_t data_type_rep, schema_t *schema)
{
    return _attr_create(name, data_type, data_type_rep,
                        (ATTR_FLAGS) { .autoinc = 0, .foreign = 0, .nullable = 0, .primary = 0, .unique = 0 },
                        schema);
}

bool attr_isstring(const attr_t *attr)
{
    return (attr == NULL ? false : (attr->type == FT_CHAR));
}

DEFINE_ATTRIBUTE_CREATE(bool, FT_BOOL)

DEFINE_ATTRIBUTE_CREATE(int8, FT_INT8)

DEFINE_ATTRIBUTE_CREATE(int16, FT_INT16)

DEFINE_ATTRIBUTE_CREATE(int32, FT_INT32)

DEFINE_ATTRIBUTE_CREATE(int64, FT_INT64)

DEFINE_ATTRIBUTE_CREATE(uint8, FT_UINT8)

DEFINE_ATTRIBUTE_CREATE(uint16, FT_UINT16)

DEFINE_ATTRIBUTE_CREATE(uint32, FT_UINT32)

DEFINE_ATTRIBUTE_CREATE(uint64, FT_UINT64)

DEFINE_ATTRIBUTE_CREATE(float32, FT_FLOAT32)

DEFINE_ATTRIBUTE_CREATE(float64, FT_FLOAT64)

DEFINE_ATTRIBUTE_ARRAY_CREATE(string, FT_CHAR)

/* internal */

DEFINE_ATTRIBUTE_CREATE(strptr, FT_STRPTR)

DEFINE_ATTRIBUTE_CREATE(attrid, FT_ATTRID)

DEFINE_ATTRIBUTE_CREATE(gridid, FT_GRIDID)

DEFINE_ATTRIBUTE_CREATE(tupleid, FT_TUPLEID)

DEFINE_ATTRIBUTE_CREATE(fragtype, FT_FRAGTYPE)

DEFINE_ATTRIBUTE_CREATE(size, FT_SIZE)

DEFINE_ATTRIBUTE_CREATE(tformat, FT_TFORMAT)



