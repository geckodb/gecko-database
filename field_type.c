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

#include <field_type.h>
#include <grid.h>

size_t gs_field_type_sizeof(enum field_type type)
{
    switch (type) {
        case FT_CHAR:
            return sizeof(CHAR);
        case FT_BOOL:
            return sizeof(BOOL);
        case FT_INT8:
            return sizeof(INT8);
        case FT_UINT8:
            return sizeof(UINT8);
        case FT_INT16:
            return sizeof(INT16);
        case FT_UINT16:
            return sizeof(UINT16);
        case FT_INT32:
            return sizeof(INT32);
        case FT_UINT32:
            return sizeof(UINT32);
        case FT_FLOAT32:
            return sizeof(FLOAT32);
        case FT_INT64:
            return sizeof(INT64);
        case FT_UINT64:
            return sizeof(UINT64);
        case FT_FLOAT64:
            return sizeof(FLOAT64);
        /* internal */
        case FT_STRPTR:
            return sizeof(STRPTR);
        case FT_ATTRID:
            return sizeof(ATTRID);
        case FT_GRIDID:
            return sizeof(GRIDID);
        case FT_TUPLEID:
            return sizeof(TUPLEID);
        case FT_FRAGTYPE:
            return sizeof(FRAGTYPE);
        case FT_SIZE:
            return sizeof(SIZE);
        case FT_TFORMAT:
            return sizeof(TFORMAT);
        default:
            perror ("Unknown type");
            abort();
    }
}

const char *gs_field_type_str(enum field_type type)
{
    switch (type) {
        case FT_CHAR:
            return "char";
        case FT_BOOL:
            return "bool";
        case FT_INT8:
            return "s8";
        case FT_UINT8:
            return "u8";
        case FT_INT16:
            return "s16";
        case FT_UINT16:
            return "u16";
        case FT_INT32:
            return "s32";
        case FT_UINT32:
            return "u32";
        case FT_FLOAT32:
            return "float32";
        case FT_INT64:
            return "s64";
        case FT_UINT64:
            return "u64";
        case FT_FLOAT64:
            return "float64";
            /* internal */
        case FT_STRPTR:
            return "string";
        case FT_ATTRID:
            return "attr id";
        case FT_GRIDID:
            return "grid id";
        case FT_TUPLEID:
            return "tuple id";
        case FT_FRAGTYPE:
            return "frag type";
        case FT_SIZE:
            return "size";
        case FT_TFORMAT:
            return "tuple format";
        default:
            perror ("Unknown type");
            abort();
    }
}