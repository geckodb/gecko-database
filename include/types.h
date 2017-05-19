// Column data types
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

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    type_internal_undef,
    type_internal_tuple_id,

    type_single_char,
    type_single_bool,
    type_single_uint8,
    type_single_uint16,
    type_single_uint32,
    type_single_uint64,
    type_single_int8,
    type_single_int16,
    type_single_int32,
    type_single_int64,
    type_single_float,
    type_single_double,
    type_multi_fixed,
    type_multi_variable
} data_type;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t type_sizeof(data_type type);

bool type_is_fixed_size(data_type type);

