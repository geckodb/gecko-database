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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <defs.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum mdb_require_relation { RR_LESS_THAN, RR_LESS_EQUAL, RR_EQUAL, RR_GREATER_EQUAL, RR_GREATER };

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool mdb_require_non_null(const void *ptr);

void *mdb_require_malloc(size_t size);

bool mdb_require_less_than(const void *lhs, const void *rhs);

bool mdb_require_relation(const void *lhs, enum mdb_require_relation relation, const void *rhs,
                          int (*comp)(const void *lhs, const void *rhs));

bool mdb_require_relation_size_t(size_t lhs, enum mdb_require_relation relation, size_t rhs);

bool mdb_require_true(bool value);