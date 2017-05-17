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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <types.h>
#include <error.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

size_t mdb_type_sizeof(enum mdb_type type)
{
    switch (type) {
        case TYPE_BOOLEAN: case TYPE_BYTE: case TYPE_CHAR: case TYPE_VAR_STRING: case TYPE_FIX_STRING: return 1;
        case TYPE_SHORT:                                                                               return 2;
        case TYPE_INTEGER: case TYPE_FLOAT:                                                            return 4;
        case TYPE_LONG: case TYPE_DOUBLE: case TYPE_TID:                                               return 8;
        default: {
            error_set_last(EC_UNKNOWNTYPE);
            return 0;
        }
    }
}