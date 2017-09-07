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

#include <stdinc.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    constraint_less,
    constraint_less_equal,
    constraint_equal,
    constraint_greater_equal,
    constraint_greater
} relation_constraint;

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define REQUIRE_NONNULL(x)                                                                                             \
    panic_if((x == NULL), BADARG, "parameter '" to_string(x) "' is null");

#define REQUIRE_IMPL(fun)                                                                                              \
    panic_if((fun == NULL), BADCALL, "illegal call to pure virtual function '" to_string(fun) "'");

#define REQUIRE(expr, msg)                                                                                             \
    panic_if((!(expr)), BADEXPR, msg);

#define REQUIRE_NONZERO(value)                                                                                         \
    REQUIRE((value != 0), "Value must be non-zero")

#define require_not_zero(value)                                                                                        \
    REQUIRE(value > 0, to_string(value) " is not allowed to be zero")

#define REQUIRE_MALLOC(size)                                                                                      \
    ({                                                                                                                 \
        void *block = malloc(size);                                                                                    \
        panic_if((block == NULL), BADMALLOC, "request to allocate '" to_string(size) "' bytes failed");                \
        block;                                                                                                         \
    })

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool require_less_than(const void *lhs, const void *rhs);

bool require_constraint(const void *lhs, relation_constraint constraint, const void *rhs,
                        int (*comp)(const void *lhs, const void *rhs));

bool require_constraint_size_t(size_t lhs, relation_constraint constraint, size_t rhs);