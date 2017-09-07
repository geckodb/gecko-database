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

#include <stdinc.h>
#include <require.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static int _comp_size_t(const void *lhs, const void *rhs);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool require_less_than(const void *lhs, const void *rhs)
{
    bool is_less_than = (lhs < rhs);
    error_if(!is_less_than, err_corrupted);
    return is_less_than;
}

bool _check_expected_true(bool expr)
{
    if (!expr) {
        error(err_constraint_violated);
    }
    return expr;
}

bool require_constraint(const void *lhs, relation_constraint constraint, const void *rhs,
                        int (*comp)(const void *, const void *))
{
    REQUIRE_NONNULL(lhs)
    REQUIRE_NONNULL(rhs)
    REQUIRE_NONNULL(comp)
    int result = comp(lhs, rhs);
    switch (constraint) {
        case constraint_less:          return _check_expected_true(result < 0);
        case constraint_less_equal:    return _check_expected_true(result <= 0);
        case constraint_equal:         return _check_expected_true(result == 0);
        case constraint_greater_equal: return _check_expected_true(result >= 0);
        case constraint_greater:       return _check_expected_true(result > 0);
        default: {
            error(err_internal);
            return false;
        }
    }
    return false;
}

bool require_constraint_size_t(size_t lhs, relation_constraint constraint, size_t rhs)
{
    return require_constraint(&lhs, constraint, &rhs, _comp_size_t);
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

int _comp_size_t(const void *lhs, const void *rhs)
{
    assert(lhs != NULL && rhs != NULL);
    size_t a = *((size_t *) lhs);
    size_t b = *((size_t *) rhs);
    return (a < b ? - 1 : (a > b ? 1 : 0));
}