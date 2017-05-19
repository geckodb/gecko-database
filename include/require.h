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

typedef enum {
    constraint_less_than,
    constraint_less,
    constraint_less_equal,
    constraint_equal,
    constraint_greater_equal,
    constraint_greater
} relation_constraint;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool require_non_null(const void *ptr);

bool require_non_zero(int64_t value);

void *require_malloc(size_t size);

bool require_less_than(const void *lhs, const void *rhs);

bool require_constraint(const void *lhs, relation_constraint constraint, const void *rhs,
                        int (*comp)(const void *lhs, const void *rhs));

bool require_constraint_size_t(size_t lhs, relation_constraint constraint, size_t rhs);