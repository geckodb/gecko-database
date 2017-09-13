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
#include <containers/vec.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    char *frag_name;
    vec_t *attr;
} schema_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

schema_t *schema_new(const char *table_name);
void gs_schema_delete(schema_t *schema);
schema_t *schema_subset(schema_t *super, const attr_id_t *indicies, size_t nindicies);
schema_t *schema_cpy(const schema_t *schema);
const struct attr_t *schema_attr_by_id(const schema_t *schema, attr_id_t attr_id);
const struct attr_t *schema_attr_by_name(const schema_t *schema, const char *name);
size_t schema_attr_size_by_id(schema_t *schema, attr_id_t attr_id);
size_t schema_num_attributes(const schema_t *schema);
const attr_id_t *schema_attributes(const schema_t *schema);
enum field_type schema_attr_type(schema_t *schema, attr_id_t id);
void schema_print(FILE *file, schema_t *schema);