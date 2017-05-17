// An implementation of table schemas
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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

#include <defs.h>
#include <containers/vector.h>
#include <storage/attribute.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct
{
    mdb_vector *attributes;
} mdb_schema;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

mdb_schema *mdb_schema_alloc(size_t attribute_capacity);

bool mdb_schema_free(mdb_schema *schema);

bool mdb_schema_comp(const mdb_schema *lhs, const mdb_schema *rhs);

bool mdb_schema_add(const mdb_schema *schema, const mdb_attribute *attr);

bool mdb_schema_set(const mdb_schema *schema, size_t attr_idx, const mdb_attribute *attr);

enum mdb_type mdb_schema_get_type(const mdb_schema *schema, size_t attr_idx);

const mdb_attribute *mdb_schema_get_attr_by_name(const mdb_schema *schema, const char *name);

const char *mdb_schema_get_attribute_name(const mdb_schema *schema, const size_t attr_idx);