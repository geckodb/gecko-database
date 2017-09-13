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

enum field_type {
    FT_BOOL     =  0,
    FT_INT8     =  1,
    FT_INT16    =  2,
    FT_INT32    =  3,
    FT_INT64    =  4,
    FT_UINT8    =  5,
    FT_UINT16   =  6,
    FT_UINT32   =  7,
    FT_UINT64   =  8,
    FT_FLOAT32  =  9,
    FT_FLOAT64  = 10,
    FT_CHAR     = 11,
    /* internal */
    FT_STRPTR   = 100,
    FT_ATTRID   = 101,
    FT_GRIDID   = 102,
    FT_TUPLEID  = 103,
    FT_FRAGTYPE = 104,
    FT_SIZE     = 105,
    FT_TFORMAT  = 106,
};

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_field_type_sizeof(enum field_type type);

const char *gs_field_type_str(enum field_type type);