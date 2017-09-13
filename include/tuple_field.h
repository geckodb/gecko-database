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
#include <tuple.h>
#include <tuplet_field.h>
#include <grid.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct tuple_field_t
{
    const grid_t *grid;
    attr_id_t table_attr_id;
    attr_id_t grid_attr_id;
    tuple_t *tuple;
    tuplet_t tuplet;
    tuplet_field_t tuplet_field;
} tuple_field_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void tuple_field_open(tuple_field_t *field, tuple_t *tuple);
void tuple_field_seek(tuple_field_t *tuple_field, tuple_t *tuple, attr_id_t table_attr_id);
void tuple_field_next(tuple_field_t *field);
void tuple_field_write(tuple_field_t *field, const void *data);
const void *tuple_field_read(tuple_field_t *field);
