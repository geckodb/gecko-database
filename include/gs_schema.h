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
#include <gecko-commons/containers/gs_vec.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    char *frag_name;
    gs_vec_t *attr;
} gs_schema_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_schema_t *gs_schema_new(const char *table_name);
void gs_schema_delete(gs_schema_t *schema);
gs_schema_t *gs_schema_subset(gs_schema_t *super, const gs_attr_id_t *indicies, size_t nindicies);
gs_schema_t *gs_schema_cpy(const gs_schema_t *schema);
const struct gs_attr_t *gs_schema_attr_by_id(const gs_schema_t *schema, gs_attr_id_t attr_id);
const struct gs_attr_t *gs_schema_attr_by_name(const gs_schema_t *schema, const char *name);
size_t gs_schema_attr_size_by_id(gs_schema_t *schema, gs_attr_id_t attr_id);
size_t gs_schema_num_attributes(const gs_schema_t *schema);
const gs_attr_id_t *gs_schema_attributes(const gs_schema_t *schema);
enum gs_field_type_e gs_schema_attr_type(gs_schema_t *schema, gs_attr_id_t id);
void gs_schema_print(FILE *file, gs_schema_t *schema);