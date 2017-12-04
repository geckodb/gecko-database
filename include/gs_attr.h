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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gecko-commons/gecko-commons.h>
#include <gs_schema.h>
#include <gs_field_type.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define DECLARE_ATTRIBUTE_CREATE(type_name,internal_type)                                                              \
gs_attr_id_t attr_create_##type_name(const char *name, gs_schema_t *schema);

#define DECLARE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                        \
gs_attr_id_t attr_create_##type_name(const char *name, size_t length, gs_schema_t *schema);

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_attr_t {
    size_t id;
    char name[ATTR_NAME_MAXLEN];
    enum gs_field_type_e type;
    size_t type_rep;
    gs_attr_flags_e flags;
    size_t str_format_mlen;
    unsigned char checksum[MD5_DIGEST_LENGTH];
} gs_attr_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_attr_id_t gs_attr_create(const char *name, enum gs_field_type_e data_type, size_t data_type_rep, gs_schema_t *schema);
const char *gs_attr_name(const struct gs_attr_t *attr);
bool gs_attr_isstring(const gs_attr_t *attr);
size_t gs_attr_str_max_len(gs_attr_t *attr);
enum gs_field_type_e gs_attr_type(const gs_attr_t *attr);
const gs_attr_t *gs_attr_cpy(const gs_attr_t *template, gs_schema_t *new_owner);
size_t gs_attr_total_size(const struct gs_attr_t *attr);

DECLARE_ATTRIBUTE_CREATE(bool, FT_BOOL)
DECLARE_ATTRIBUTE_CREATE(int8, FT_INT8)
DECLARE_ATTRIBUTE_CREATE(int16, FT_INT16)
DECLARE_ATTRIBUTE_CREATE(int32, FT_INT32)
DECLARE_ATTRIBUTE_CREATE(int64, FT_INT64)
DECLARE_ATTRIBUTE_CREATE(uint8, FT_UINT8)
DECLARE_ATTRIBUTE_CREATE(uint16, FT_UINT16)
DECLARE_ATTRIBUTE_CREATE(uint32, FT_UINT32)
DECLARE_ATTRIBUTE_CREATE(uint64, FT_UINT64)
DECLARE_ATTRIBUTE_CREATE(float32, FT_FLOAT32)
DECLARE_ATTRIBUTE_CREATE(float64, FT_FLOAT64)
DECLARE_ATTRIBUTE_ARRAY_CREATE(string, FT_CHAR)

// I N T E R N A L -----------------------------------------------------------------------------------------------------

DECLARE_ATTRIBUTE_CREATE(strptr, FT_STRPTR)
DECLARE_ATTRIBUTE_CREATE(attrid, FT_ATTRID)
DECLARE_ATTRIBUTE_CREATE(gridid, FT_GRIDID)
DECLARE_ATTRIBUTE_CREATE(tupleid, FT_TUPLEID)
DECLARE_ATTRIBUTE_CREATE(fragtype, FT_FRAGTYPE)
DECLARE_ATTRIBUTE_CREATE(size, FT_SIZE)
DECLARE_ATTRIBUTE_CREATE(tformat, FT_TFORMAT)
DECLARE_ATTRIBUTE_CREATE(rectype, FT_RECTYPE)
DECLARE_ATTRIBUTE_CREATE(reltype, FT_RELTYPE)



