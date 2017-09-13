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

#include <stdinc.h>
#include <containers/vector.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    char *frag_name;
    vector_t *attr;
} schema_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

schema_t *gs_schema_create(const char *table_name);

schema_t *gs_schema_subset(schema_t *super, const attr_id_t *indicies, size_t nindicies);

void gs_schema_free(schema_t *schema);

schema_t *gs_schema_cpy(const schema_t *schema);

const struct attr_t *gs_schema_attr_by_id(const schema_t *schema, attr_id_t attr_id);

const struct attr_t *gs_schema_attr_by_name(const schema_t *schema, const char *name);

size_t gs_schema_attr_size_by_id(schema_t *schema, attr_id_t attr_id);

size_t gs_schema_num_attributes(const schema_t *schema);

const attr_id_t *gs_schema_attributes(const schema_t *schema);

enum field_type gs_schema_attr_type(schema_t *schema, attr_id_t id);

void gs_schema_print(FILE *file, schema_t *schema);