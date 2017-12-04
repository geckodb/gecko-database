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

#include <gecko-commons/containers/gs_vec.h>

#include <gs_attr.h>
#include <gs_tuplet_field.h>
#include <gs_frag.h>
#include <gs_schema.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define DEFINE_ATTRIBUTE_CREATE(type_name,internal_type)                                                               \
gs_attr_id_t attr_create_##type_name(const char *name, gs_schema_t *schema) {                                          \
    return gs_attr_create(name, internal_type, 1, schema);                                                             \
}

#define DEFINE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                         \
gs_attr_id_t attr_create_##type_name(const char *name, size_t length, gs_schema_t *schema) {                           \
    return gs_attr_create(name, internal_type, length, schema);                                                        \
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_attr_id_t _attr_create(const char *name, enum gs_field_type_e data_type, size_t data_type_rep, gs_attr_flags_e attr_flags, gs_schema_t *schema)
{
    panic_if((name == NULL || schema == NULL || strlen(name) + 1 > ATTR_NAME_MAXLEN), BADARG,
             "null pointer or attribute name limit exceeded");

    gs_attr_t attr = {
            .id     = schema->attr->num_elements,
            .type     = data_type,
            .type_rep = data_type_rep,
            .flags    = attr_flags,
            .str_format_mlen   = 0,
    };

    strcpy(attr.name, name);
    memset(attr.checksum, 0, MD5_DIGEST_LENGTH);
    gs_vec_pushback(schema->attr, 1, &attr);
    return attr.id;
}

const char *gs_attr_name(const struct gs_attr_t *attr)
{
    assert (attr);
    return attr->name;
}

size_t gs_attr_str_max_len(gs_attr_t *attr)
{
    assert (attr);
    return attr->str_format_mlen;
}

enum gs_field_type_e gs_attr_type(const gs_attr_t *attr)
{
    assert (attr);
    return attr->type;
}

const gs_attr_t *gs_attr_cpy(const gs_attr_t *template, gs_schema_t *new_owner)
{
    assert (template);
    assert(new_owner);
    gs_attr_id_t id = _attr_create(template->name, template->type, template->type_rep, template->flags, new_owner);
    return gs_schema_attr_by_id(new_owner, id);
}

size_t gs_attr_total_size(const struct gs_attr_t *attr)
{
    return attr->type_rep * gs_field_type_sizeof(attr->type);
}

gs_attr_id_t gs_attr_create(const char *name, enum gs_field_type_e data_type, size_t data_type_rep, gs_schema_t *schema)
{
    return _attr_create(name, data_type, data_type_rep,
                        (gs_attr_flags_e) { .autoinc = 0, .foreign = 0, .nullable = 0, .primary = 0, .unique = 0 },
                        schema);
}

bool gs_attr_isstring(const gs_attr_t *attr)
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


// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------


DEFINE_ATTRIBUTE_CREATE(strptr, FT_STRPTR)

DEFINE_ATTRIBUTE_CREATE(attrid, FT_ATTRID)

DEFINE_ATTRIBUTE_CREATE(gridid, FT_GRIDID)

DEFINE_ATTRIBUTE_CREATE(tupleid, FT_TUPLEID)

DEFINE_ATTRIBUTE_CREATE(fragtype, FT_FRAGTYPE)

DEFINE_ATTRIBUTE_CREATE(size, FT_SIZE)

DEFINE_ATTRIBUTE_CREATE(tformat, FT_TFORMAT)

DEFINE_ATTRIBUTE_CREATE(reltype, FT_RELTYPE)



