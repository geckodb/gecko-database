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
            result->flags = attribute_flags_to_flag_t(flags);
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

attribute_flags_t attribute_default_flags()
{
    attribute_flags_t flags = {
        .non_null = false,
        .primary_key = false,
        .auto_inc = false,
        .unique = false
    };
    return flags;
}

attribute_flags attribute_flags_t_to_flag(attribute_flags_t flags)
{
    attribute_flags result = 0;
    if (flags.auto_inc)
        result |= attribute_autoinc;
    if (flags.non_null)
        result |= attribute_nonnull;
    if (flags.primary_key)
        result |= attribute_primary;
    if (flags.unique)
        result |= attribute_unique;
    return result;
}

attribute_flags_t attribute_flags_to_flag_t(attribute_flags flags)
{
    attribute_flags_t result = {
            .auto_inc    = ((flags & attribute_autoinc) == attribute_autoinc),
            .non_null    = ((flags & attribute_nonnull) == attribute_nonnull),
            .primary_key = ((flags & attribute_primary) == attribute_primary),
            .unique      = ((flags & attribute_unique)  == attribute_unique)
    };
    return result;
}

void attribute_print(FILE *file, const attribute_t *attr)
{
    if (require_non_null(file) && require_non_null(attr)) {
        fprintf(file, "attribute(name='%s', type='%s', length=%zu, flags='",
                attr->name, type_to_string(attr->type), attr->length);
        attribute_flags_print(file, attr->flags);
        fprintf(file, "')");
    }
}

void attribute_flags_print(FILE *file, attribute_flags_t flags)
{
    if (require_non_null(file)) {
        if (flags.auto_inc)
            fprintf(file, "attribute_autoinc ");
        if (flags.non_null)
            fprintf(file, "attribute_nonnull ");
        if (flags.primary_key)
            fprintf(file, "attribute_primary ");
        if (flags.unique)
            fprintf(file, "attribute_unique ");
    }
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

