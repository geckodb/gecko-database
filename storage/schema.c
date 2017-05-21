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

static bool _check_valid_schema(const schema_t *);
static bool _check_in_bounds(const schema_t *, size_t);
static bool _free_attribute(void *capture, void *, void *);
static bool _comp_attribute(const void *a, const void *b);
static bool _find_attribute(void *capture, void *from, void *to);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

schema_t *schema_create(size_t attribute_capacity)
{
    schema_t *result;
    if ((result = malloc (sizeof(schema_t))) == NULL) {
        error(err_bad_malloc);
    } else {
        result->attributes = vector_create(sizeof(attribute_t *), attribute_capacity);
        result->is_fixed_size = true;
        result->varseq_length = 0;
    }
    return result;
}

bool schema_is_fixed_size(const schema_t *schema)
{
    size_t attr_idx = schema_num_attributes(schema);
    while (attr_idx--) {
        data_type type = schema_get(schema, attr_idx);
        if (!type_is_fixed_size(type))
            return false;
    }
    return true;
}

schema_t *schema_cpy(const schema_t *proto)
{
    schema_t *result = NULL;
    if ((require_non_null(proto)) && (require_non_null(proto->attributes)) &&
       ((result = require_malloc(sizeof(schema_t))))) {
        result->attributes = vector_cpy(proto->attributes);
        result->is_fixed_size = proto->is_fixed_size;
    }
    return result;
}

bool schema_free(schema_t *schema)
{
    bool result = _check_valid_schema(schema);
    if (result) {
        if (schema->attributes != NULL) {
            vector_foreach(NULL, schema->attributes, _free_attribute);
            vector_free(schema->attributes);
            schema->attributes = NULL;
        }
        free (schema);
    }
    return result;
}

bool schema_comp(const schema_t *lhs, const schema_t *rhs)
{
    return (_check_valid_schema(lhs) && _check_valid_schema(rhs) &&
            vector_comp(lhs->attributes, rhs->attributes, _comp_attribute) && lhs->is_fixed_size == rhs->is_fixed_size);
}

bool schema_add(schema_t *schema, const attribute_t *attr)
{
    return require_non_null(schema) ? schema_set(schema, schema->attributes->num_elements, attr) : false;
}

bool schema_set(schema_t *schema, size_t attr_idx, const attribute_t *attr)
{
    bool result = (_check_valid_schema(schema) && require_non_null(attr));
    if (result) {
        attribute_t *cpy = attribute_cpy(attr);
        schema->is_fixed_size &= type_is_fixed_size(attr->type);
        vector_set(schema->attributes, attr_idx, 1, &cpy);
    }
    return result;
}

size_t schema_num_attributes(const schema_t *schema)
{
    size_t result = require_non_null(schema);
    if (result) {
        return (schema->attributes->num_elements);
    }
    return result;
}

data_type schema_get(const schema_t *schema, size_t attr_idx)
{
    if (_check_valid_schema(schema) && _check_in_bounds(schema, attr_idx)) {
        return (*(((const attribute_t**) vector_get(schema->attributes)) + attr_idx))->type;
    } else return type_internal_undef;
}

typedef struct
{
    const char *needle;
    attribute_t *result;
} _attr_by_name_params;

const attribute_t *schema_get_by_name(const schema_t *schema, const char *name)
{
    if (require_non_null(schema) && require_non_null(name)) {
        _attr_by_name_params capture = {
            .needle = name,
            .result = NULL
        };
        if (!vector_foreach(schema->attributes, &capture, _find_attribute)) {
            error_print();
        }
        return capture.result;
    } return NULL;
}

const char *schema_get_attr_name(const schema_t *schema, const size_t attr_idx)
{
    if (_check_valid_schema(schema) && _check_in_bounds(schema, attr_idx)) {
        return (*(((const attribute_t **) vector_get(schema->attributes)) + attr_idx))->name;
    } else return NULL;
}

attribute_flags_t schema_get_attr_flags(const schema_t *schema, const size_t attr_idx)
{
    attribute_flags_t default_flags = attribute_default_flags();
    if (_check_valid_schema(schema) && _check_in_bounds(schema, attr_idx)) {
        return ((const attribute_t *) vector_get(schema->attributes))->flags;
    } else return default_flags;
}

void schema_print(FILE *file, const schema_t *schema)
{
    if (require_non_null(file) && require_non_null(schema)) {
        size_t attr_idx = schema->attributes->num_elements;
        fprintf(file, "schema (fixed_size=%d, num_attributes=%zu, attributes=[", schema->is_fixed_size, attr_idx);
        while (attr_idx--) {
            const attribute_t *attr = schema_get_by_id(schema, attr_idx);
            attribute_print(file, attr);
            if (attr_idx > 0)
                fprintf(file, ", ");
        }
        fprintf(file, "]");
    }
}

const attribute_t *schema_get_by_id(const schema_t *schema, size_t attr_idx)
{
    if (_check_valid_schema(schema) && _check_in_bounds(schema, attr_idx)) {
        return *(((const attribute_t **) vector_get(schema->attributes)) + attr_idx);
    } else return NULL;
}

bool _is_fixed_schema(const schema_t *fixed_schema)
{
    bool is_fixed = fixed_schema->is_fixed_size;
    error_if(!is_fixed, err_illegal_args);
    return is_fixed;
}

size_t schema_get_tuple_size(const schema_t *fixed_schema)
{
    size_t total_size = 0;
    if (require_non_null(fixed_schema) && _is_fixed_schema(fixed_schema)) {
        size_t attr_idx = schema_num_attributes(fixed_schema);
        while (attr_idx--) {
            const attribute_t *attr = schema_get_by_id(fixed_schema, attr_idx);
            total_size += type_sizeof(attr->type) * attr->length;
        }
    }
    return total_size;
}

size_t schema_get_varseq_size(const schema_t *fixed_schema)
{
    size_t total_size = 0;
    if (require_non_null(fixed_schema) && _is_fixed_schema(fixed_schema)) {
        total_size = fixed_schema->varseq_length;
    }
    return total_size;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool _check_valid_schema(const schema_t *s)
{
    bool result = ((s != NULL) && (s->attributes != NULL));
    error_if(!result, err_illegal_args);
    return result;
}

bool _check_in_bounds(const schema_t *s, size_t attribute_idx)
{
    bool result = (attribute_idx < s->attributes->num_elements);
    error_if(!result, err_out_of_bounds);
    return result;
}

bool _free_attribute(void *capture, void *begin, void *end)
{
    bool result = require_non_null(begin) && require_non_null(end) && require_less_than(begin, end);
    for (attribute_t **it = (attribute_t **) begin; it < (attribute_t **) end; it++)
        result &= attribute_free(*it);
    return result;
}

bool _comp_attribute(const void *a, const void *b)
{
    return attribute_comp((attribute_t *) a, (attribute_t *) b);
}

bool _find_attribute(void *capture, void *from, void *to)
{
    bool result = (require_non_null(capture) && require_non_null(from) && require_non_null(to));
    if (result) {
        attribute_t **begin = (attribute_t **) from, **end = (attribute_t **) to;
        _attr_by_name_params *args = (_attr_by_name_params *) capture;

        for (attribute_t **it = begin; it < end; it++) {
            const char *it_name = (*it)->name;
            if (!require_non_null(it_name))
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