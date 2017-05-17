// An implementation of table schemas
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
#include <storage/schema.h>
#include <containers/vector.h>
#include <require.h>
#include <storage/attribute.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static bool check_valid_schema(const mdb_schema *);
static bool check_attribute_idx_in_bounds(const mdb_schema *, size_t);
static bool free_attribute_ptr(void *capture, void *, void *);
static bool _comp_attribute(const void *a, const void *b);
static bool _find_attr(void *capture, void *from, void *to);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

mdb_schema *mdb_schema_alloc(size_t attribute_capacity)
{
    mdb_schema *result;
    if ((result = malloc (sizeof(mdb_schema))) == NULL) {
        error_set_last(EC_BADMALLOC);
    } else {
        result->attributes = mdb_vector_alloc(sizeof(mdb_attribute *), attribute_capacity);
    }
    return result;
}

bool mdb_schema_free(mdb_schema *schema)
{
    bool result = check_valid_schema(schema);
    if (result) {
        if (schema->attributes != NULL) {
            mdb_vector_foreach(NULL, schema->attributes, free_attribute_ptr);
            mdb_vector_free(schema->attributes);
            schema->attributes = NULL;
        }
        free (schema);
    }
    return result;
}

bool mdb_schema_comp(const mdb_schema *lhs, const mdb_schema *rhs)
{
    return (check_valid_schema(lhs) && check_valid_schema(rhs) &&
            mdb_vector_comp(lhs->attributes, rhs->attributes, _comp_attribute));
}

bool mdb_schema_add(const mdb_schema *schema, const mdb_attribute *attr)
{
    return mdb_require_non_null(schema) ? mdb_schema_set(schema, schema->attributes->num_elements, attr) : false;
}

bool mdb_schema_set(const mdb_schema *schema, size_t attr_idx, const mdb_attribute *attr)
{
    bool result = (check_valid_schema(schema) && mdb_require_non_null(attr));
    if (result) {
        mdb_attribute *cpy = mdb_attribute_cpy(attr);
        mdb_vector_set(schema->attributes, attr_idx, 1, &cpy);
    }
    return result;
}

enum mdb_type mdb_schema_get_type(const mdb_schema *schema, size_t attr_idx)
{
    if (check_valid_schema(schema) && check_attribute_idx_in_bounds(schema, attr_idx)) {
        return ((const mdb_attribute*) mdb_vector_get(schema->attributes))->type;
    } else return TYPE_UNDEFINED;
}

typedef struct
{
    const char *needle;
    mdb_attribute *result;
} _attr_by_name_params;

const mdb_attribute *mdb_schema_get_attr_by_name(const mdb_schema *schema, const char *name)
{
    if (mdb_require_non_null(schema) && mdb_require_non_null(name)) {
        _attr_by_name_params capture = {
            .needle = name,
            .result = NULL
        };
        if (!mdb_vector_foreach(schema->attributes, &capture, _find_attr)) {
            error_print();
        }
        return capture.result;
    } return NULL;
}

const char *mdb_schema_get_attribute_name(const mdb_schema *schema, const size_t attr_idx)
{
    if (check_valid_schema(schema) && check_attribute_idx_in_bounds(schema, attr_idx)) {
        return ((const mdb_attribute *) mdb_vector_get(schema->attributes))->name;
    } else return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool check_valid_schema(const mdb_schema *s)
{
    bool result = ((s != NULL) && (s->attributes != NULL));
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

bool check_attribute_idx_in_bounds(const mdb_schema *s, size_t attribute_idx)
{
    bool result = (attribute_idx < s->attributes->num_elements);
    error_set_last_if(!result, EC_OUTOFBOUNDS);
    return result;
}

bool free_attribute_ptr(void *capture, void *begin, void *end)
{
    bool result = mdb_require_non_null(begin) && mdb_require_non_null(end) && mdb_require_less_than(begin, end);
    for (mdb_attribute **it = (mdb_attribute **) begin; it < (mdb_attribute **) end; it++)
        result &= mdb_attribute_free(*it);
    return result;
}

bool _comp_attribute(const void *a, const void *b)
{
    return mdb_attribute_comp((mdb_attribute *) a, (mdb_attribute *) b);
}

bool _find_attr(void *capture, void *from, void *to)
{
    bool result = (mdb_require_non_null(capture) && mdb_require_non_null(from) && mdb_require_non_null(to));
    if (result) {
        mdb_attribute **begin = (mdb_attribute **) from, **end = (mdb_attribute **) to;
        _attr_by_name_params *args = (_attr_by_name_params *) capture;

        for (mdb_attribute **it = begin; it < end; it++) {
            const char *it_name = (*it)->name;
            if (!mdb_require_non_null(it_name))
                return false;
            else {
                if (strcmp(it_name, args->needle) == 0) {
                    args->result = *it;
                    break;
                }
            }
        }
    }
    return true;
}