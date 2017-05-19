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

static bool _check_create_args(const char *name, data_type type, size_t length);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

attribute_t *attribute_create(data_type type, size_t length, const char *name, attribute_flags flags)
{
    attribute_t *result = NULL;
    if (_check_create_args(name, type, length)) {
        if ((result = malloc (sizeof(attribute_t))) == NULL) {
            error(err_bad_malloc);
        } else {
            result->name = strdup(name);
            result->type = type;
            result->length = length;
            result->flags.auto_inc    = ((flags & attribute_autoinc)     == attribute_autoinc);
            result->flags.non_null    = ((flags & attribute_nonnull)    == attribute_nonnull);
            result->flags.primary_key = ((flags & attribute_primary) == attribute_primary);
            result->flags.unique      = ((flags & attribute_unique)      == attribute_unique);

            result->type = type;
        }
    }
    return result;
}

bool attribute_free(attribute_t *attr)
{
    bool non_null = require_non_null(attr);
    if (non_null) {
        free ((void *) attr->name);
        free (attr);
    }
    return non_null;
}

bool attribute_comp(const attribute_t *lhs, const attribute_t *rhs)
{
    return (require_non_null(lhs) && require_non_null(rhs)) &&
           (lhs->type == rhs->type) &&
           (lhs->flags.primary_key == rhs->flags.primary_key) &&
           (lhs->flags.non_null == rhs->flags.non_null) &&
           (lhs->flags.auto_inc == rhs->flags.auto_inc) &&
           (lhs->length == rhs->length) &&
           (strcmp(lhs->name, rhs->name) == 0);
}

attribute_t *attribute_cpy(const attribute_t *proto)
{
    if (require_non_null(proto)) {
        attribute_t *cpy = malloc (sizeof(attribute_t));
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

bool _check_create_args(const char *name, data_type type, size_t length)
{
    bool result = (((name != NULL) && (strlen(name)) > 0) &&
                   ((type == type_multi_fixed) || (length == 1)));
    error_if(result, err_illegal_args);
    return result;
}

