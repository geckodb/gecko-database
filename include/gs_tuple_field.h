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

#include <gs_tuple.h>
#include <gs_tuplet_field.h>
#include <gs_grid.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_tuple_field_t
{
    const gs_grid_t *grid;
    gs_attr_id_t table_attr_id;
    gs_attr_id_t grid_attr_id;
    gs_tuple_t *tuple;
    gs_tuplet_t tuplet;
    gs_tuplet_field_t tuplet_field;
} gs_tuple_field_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_tuple_field_open(gs_tuple_field_t *field, gs_tuple_t *tuple);
void gs_tuple_field_seek(gs_tuple_field_t *tuple_field, gs_tuple_t *tuple, gs_attr_id_t table_attr_id);
void gs_tuple_field_next(gs_tuple_field_t *field);
void gs_tuple_field_write(gs_tuple_field_t *field, const void *data);
const void *gs_tuple_field_read(gs_tuple_field_t *field);
