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
    vector_t *attributes;
    bool is_fixed_size;
} schema_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

schema_t *schema_create(size_t attribute_capacity);

bool schema_is_fixed_size(schema_t *schema);

schema_t *schema_cpy(schema_t *proto);

bool schema_free(schema_t *schema);

bool schema_comp(const schema_t *lhs, const schema_t *rhs);

bool schema_add(schema_t *schema, const attribute_t *attr);

bool schema_set(schema_t *schema, size_t attr_idx, const attribute_t *attr);

size_t schema_num_attributes(const schema_t *schema);

data_type schema_get(const schema_t *schema, size_t attr_idx);

const attribute_t *schema_get_by_name(const schema_t *schema, const char *name);

const char *schema_get_attr_name(const schema_t *schema, const size_t attr_idx);