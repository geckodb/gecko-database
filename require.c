// Handy set of helper functions to guarantee assertions
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

#include <require.h>
#include <error.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool mdb_require_non_null(const void *ptr)
{
    bool is_non_null = (ptr != NULL);
    error_set_last_if(!is_non_null, EC_NULLPOINTER);
    return is_non_null;
}

bool mdb_require_less_than(const void *lhs, const void *rhs)
{
    bool is_less_than = (lhs < rhs);
    error_set_last_if(!is_less_than, EC_CORRUPTEDORDER);
    return is_less_than;
}