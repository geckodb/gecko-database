// An implementation of a tables attribute data structure
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

#include <error.h>
#include <require.h>
#include <storage/attribute.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static bool _check_alloc_args(const char *name, enum mdb_type type, size_t length);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

mdb_attribute *mdb_attribute_alloc(enum mdb_type type, size_t length, const char *name, enum mdb_attr_flags flags)
{
    mdb_attribute *result = NULL;
    if (_check_alloc_args(name, type, length)) {
        if ((result = malloc (sizeof(mdb_attribute))) == NULL) {
            error_set_last(EC_BADMALLOC);
        } else {
            result->name = strdup(name);
            result->type = type;
            result->length = length;
            result->flags.auto_inc    = ((flags & AF_AUTOINC)     == AF_AUTOINC);
            result->flags.non_null    = ((flags & AF_NON_NULL)    == AF_NON_NULL);
            result->flags.primary_key = ((flags & AF_PRIMARY_KEY) == AF_PRIMARY_KEY);
            result->flags.unique      = ((flags & AF_UNIQUE)      == AF_UNIQUE);

            result->type = type;
        }
    }
    return result;
}

bool mdb_attribute_free(mdb_attribute *attr)
{
    bool non_null = mdb_require_non_null(attr);
    if (non_null) {
        free ((void *) attr->name);
        free (attr);
    }
    return non_null;
}

bool mdb_attribute_comp(const mdb_attribute *lhs, const mdb_attribute *rhs)
{
    return (mdb_require_non_null(lhs) && mdb_require_non_null(rhs)) &&
           (lhs->type == rhs->type) &&
           (lhs->flags.primary_key == rhs->flags.primary_key) &&
           (lhs->flags.non_null == rhs->flags.non_null) &&
           (lhs->flags.auto_inc == rhs->flags.auto_inc) &&
           (lhs->length == rhs->length) &&
           (strcmp(lhs->name, rhs->name) == 0);
}

mdb_attribute *mdb_attribute_cpy(const mdb_attribute *proto)
{
    if (mdb_require_non_null(proto)) {
        mdb_attribute *cpy = malloc (sizeof(mdb_attribute));
        cpy->name = strdup(proto->name);
        cpy->type = proto->type;
        cpy->length = proto->length;
        cpy->flags.auto_inc = proto->flags.auto_inc;
        cpy->flags.non_null = proto->flags.non_null;
        cpy->flags.primary_key = proto->flags.primary_key;
        return cpy;
    } else return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static bool _check_alloc_args(const char *name, enum mdb_type type, size_t length)
{
    bool result = (((name != NULL) && (strlen(name)) > 0) &&
                   ((type == TYPE_FIX_STRING) || (length == 1)));
    error_set_last_if(result, EC_ILLEGALARG);
    return result;
}

