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
#include <assert.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static int _comp_size_t(const void *lhs, const void *rhs);
static int _comp_bool(const void *lhs, const void *rhs);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool mdb_require_non_null(const void *ptr)
{
    bool is_non_null = (ptr != NULL);
    error_set_last_if(!is_non_null, EC_NULLPOINTER);
    return is_non_null;
}

void *mdb_require_malloc(size_t size)
{
    if (mdb_require_relation_size_t(size, RR_GREATER, 0)) {
        void *block = malloc(size);
        error_set_last_if(block == NULL, EC_BADMALLOC);
        return block;
    } else return NULL;
}

bool mdb_require_less_than(const void *lhs, const void *rhs)
{
    bool is_less_than = (lhs < rhs);
    error_set_last_if(!is_less_than, EC_CORRUPTEDORDER);
    return is_less_than;
}

bool _check_expected_true(bool expr)
{
    if (!expr) {
        error_set_last(EC_RELATIONVIOLATED);
    }
    return expr;
}

bool mdb_require_relation(const void *lhs, enum mdb_require_relation relation, const void *rhs,
                          int (*comp)(const void *, const void *))
{
    if (mdb_require_non_null(lhs) && mdb_require_non_null(rhs) && mdb_require_non_null(comp)) {
        int result = comp(lhs, rhs);
        switch (relation) {
            case RR_LESS_THAN:     return _check_expected_true(result < 0);
            case RR_LESS_EQUAL:    return _check_expected_true(result <= 0);
            case RR_EQUAL:         return _check_expected_true(result == 0);
            case RR_GREATER_EQUAL: return _check_expected_true(result >= 0);
            case RR_GREATER:       return _check_expected_true(result > 0);
            default: {
                error_set_last(EC_INTERNALERROR);
                return false;
            }
        }
    }
    return false;
}

bool mdb_require_relation_size_t(size_t lhs, enum mdb_require_relation relation, size_t rhs)
{
    return mdb_require_relation(&lhs, relation, &rhs, _comp_size_t);
}

bool mdb_require_true(bool value)
{
    bool true_val = true;
    return mdb_require_relation(&value, RR_EQUAL, &true_val, _comp_bool);
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

int _comp_bool(const void *lhs, const void *rhs)
{
    assert(lhs != NULL && rhs != NULL);
    bool a = *((bool *) lhs);
    bool b = *((bool *) rhs);
    return (a == b ? 0 : 1);
}