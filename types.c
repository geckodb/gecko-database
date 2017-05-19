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

size_t type_sizeof(data_type type)
{
    switch (type) {
        case type_bool: case type_byte: case type_char: case type_var_string: case type_fix_string: return 1;
        case type_short:                                                                            return 2;
        case type_int: case type_float:                                                             return 4;
        case type_long: case type_double: case type_tuple_id:                                       return 8;
        default: {
            error(err_bad_type);
            return 0;
        }
    }
}