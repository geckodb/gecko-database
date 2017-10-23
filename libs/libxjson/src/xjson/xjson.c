//
// Copyright (C) 2017 Marcus Pinnecke
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include <memory.h>

#include <xjson/xjson.h>

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct xjson_pool_t
{
    xjson_json_t **objects;
    xjson_size_t objects_num_elements;
    xjson_size_t objects_capacity;

    xjson_array_t **arrays;
    xjson_size_t arrays_num_elements;
    xjson_size_t arrays_capacity;

    xjson_entry_t **entires;
    xjson_size_t entires_num_elements;
    xjson_size_t entires_capacity;

    char **strings;
    xjson_size_t strings_num_elements;
    xjson_size_t strings_capacity;

    xjson_value_t **values;
    xjson_size_t values_num_elements;
    xjson_size_t values_capacity;
} xjson_pool_t;

typedef struct xjson_json_t
{
    xjson_json_t            *parent;
    xjson_entry_t          **entries;
    xjson_size_t             num_entries;
    xjson_size_t             capacity;
    xjson_pool_t            *pool;
} xjson_json_t;

typedef struct xjson_array_t
{
    xjson_entry_t           *context;
    xjson_size_t             num_entries;
    xjson_size_t             capacity;
    xjson_type_e             type;
    xjson_array_entry_t    **entries;
} xjson_array_t;

typedef struct xjson_array_entry_t
{
    xjson_size_t             idx;
    xjson_value_t           *value;

} xjson_array_entry_t;

typedef struct xjson_entry_t
{
    xjson_json_t            *context;
    char                    *key;
    xjson_value_t           *value;
} xjson_entry_t;

typedef struct xjson_value_t
{
    xjson_entry_t           *context;
    xjson_type_e             type;
    union {
        xjson_array_t       *array;
        xjson_json_t        *object;
        xjson_type_integer_t integer;
        xjson_type_double_t  decimal;
        xjson_type_string_t  string;
        xjson_type_boolean_t boolean;
    };
} xjson_value_t;

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static xjson_result_e generic_autoresize(void *base, xjson_size_t elem_size, xjson_size_t *num_entries,
                                         xjson_size_t *capacity);
static xjson_json_t *json_create(xjson_pool_t *pool);
static xjson_result_e json_autoresize(xjson_json_t *object);
static xjson_result_e array_autoresize(xjson_array_t *array);
static void json_add_entry(xjson_json_t *object, xjson_entry_t *entry);
static xjson_result_e json_add_complex(xjson_json_t **object, xjson_array_t **array, xjson_json_t *parent,
                                       const char *key, xjson_type_e complex_type, xjson_type_e array_type);
static xjson_value_t *value_create(xjson_pool_t *pool, xjson_entry_t *context, xjson_type_e type);
static xjson_entry_t *entry_create(xjson_json_t *object, xjson_type_e type, const char *key);
static xjson_array_t *array_create(xjson_entry_t *context, xjson_type_e type);
static xjson_json_t *pool_malloc_object(xjson_pool_t *pool);
static xjson_array_t *pool_malloc_array(xjson_pool_t *pool);
static xjson_entry_t *pool_malloc_entry(xjson_pool_t *pool);
static xjson_value_t *pool_malloc_value(xjson_pool_t *pool);
static xjson_array_entry_t *pool_malloc_array_entry(xjson_pool_t *pool);
static char *pool_strdup(const char *str, xjson_pool_t *pool);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

xjson_result_e xjson_pool_create(xjson_pool_t **pool)
{
    xjson_pool_t *retval;
    if ((retval = malloc(sizeof(xjson_pool_t))) != NULL) {
        retval->arrays_capacity = XJSON_POOL_ARRAY_CAPACITY_DEFAULT;
        retval->objects_capacity = XJSON_POOL_OBJ_CAPACITY_DEFAULT;
        retval->arrays = malloc(retval->arrays_capacity * sizeof(xjson_array_t));
        retval->objects = malloc(retval->objects_capacity * sizeof(xjson_json_t));
        retval->arrays_num_elements = retval->objects_num_elements = 0;
        *pool = retval;
        return ((retval->arrays != NULL && retval->objects) ? xjson_result_ok : xjson_result_malloc_err);
    }
    return xjson_result_malloc_err;
}

xjson_result_e xjson_pool_dispose(xjson_pool_t **pool)
{
    // TODO:...
    return xjson_result_malloc_err;
}

xjson_result_e xjson_json_create(xjson_json_t **json, xjson_pool_t *pool)
{
    xjson_json_t *retval = json_create(pool);
    return (retval != NULL ? xjson_result_ok : (pool != NULL ? xjson_result_failed : xjson_result_nopool));
}

xjson_result_e xjson_json_parse(xjson_json_t **json, const char *text)
{
    // TODO:...
    return xjson_result_interalerr;
}

xjson_result_e xjson_json_print(FILE *file, const xjson_json_t *json)
{
    // TODO:...
    return xjson_result_interalerr;
}

xjson_result_e value_set_primitive(xjson_value_t *value, xjson_pool_t *pool, xjson_type_e type, const void *data)
{
    switch (type) {
        case xjson_number_integer:
            memcpy(&value->integer, data, sizeof(xjson_type_integer_t));
            break;
        case xjson_number_double:
            memcpy(&value->decimal, data, sizeof(xjson_type_double_t));
            break;
        case xjson_string:
            value->string = pool_strdup(data, pool);
            break;
        case xjson_boolean:
            memcpy(&value->boolean, data, sizeof(xjson_type_boolean_t));
            break;
        case xjson_null:
            break;
        case xjson_object:
        case xjson_array:
            return xjson_result_wrongusage;
        default:
            return xjson_result_notype;
    }
    return xjson_result_ok;
}

xjson_result_e xjson_json_add_property(xjson_json_t *parent, xjson_type_e type, const char *key, const void *data)
{
    if (parent != NULL && data != NULL) {
        xjson_entry_t *entry = entry_create(parent, type, key);
        if (entry != NULL && (json_autoresize(parent) == xjson_result_ok)) {
            xjson_result_e status = value_set_primitive(entry->value, parent->pool, type, data);
            if (status == xjson_result_ok) {
                json_add_entry(parent, entry);
                return xjson_result_ok;
            } else {
                return status;
            }
        } else return xjson_result_pmalloc_err;
    } else return xjson_result_nullptr;
}

xjson_result_e xjson_json_add_object(xjson_json_t **object, xjson_json_t *parent, const char *key)
{
    return json_add_complex(object, NULL, parent, key, xjson_object, 0);
}

xjson_result_e xjson_json_add_array(xjson_array_t **array, xjson_json_t *parent, xjson_type_e type, const char *key)
{
    return json_add_complex(NULL, array, parent, key, xjson_array, type);
}

xjson_result_e xjson_array_add_property(xjson_array_t *parent, const void *data)
{
    xjson_result_e status;
    if (parent != NULL && data != NULL) {
        xjson_pool_t *pool = parent->context->context->pool;
        xjson_array_entry_t *entry = pool_malloc_array_entry(pool);
        entry->value = value_create(pool, parent->context, parent->type);
        if (((status = value_set_primitive(entry->value, pool, parent->type, data)) == xjson_result_ok) &&
                (status = array_autoresize(parent)) == xjson_result_ok) {
            entry->idx = parent->num_entries;
            parent->entries[parent->num_entries++] = entry;
            return xjson_result_ok;
        } else {
            return status;
        }
    } else return xjson_result_nullptr;
}

xjson_result_e xjson_array_add_object(xjson_json_t **object, const xjson_array_t *parent)
{
    // TODO:...
    return xjson_result_interalerr;
}

xjson_result_e xjson_array_add_array(xjson_entry_t **array, const xjson_array_t *parent)
{
    // TODO:...
    return xjson_result_interalerr;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static xjson_json_t *json_create(xjson_pool_t *pool)
{
    xjson_json_t *retval = NULL;
    if ((pool != NULL) &&
        ((retval = pool_malloc_object(pool)) != NULL) &&
        ((retval->entries = malloc(XJSON_JSON_ENTRIES_CAPACITY_DEFAULT * sizeof(xjson_entry_t *))) != NULL)) {
        retval->capacity = XJSON_JSON_ENTRIES_CAPACITY_DEFAULT;
        retval->num_entries = 0;
        retval->parent = XJSON_ROOT;
        retval->pool = pool;
    }
    return retval;
}

static xjson_result_e generic_autoresize(void *base, xjson_size_t elem_size, xjson_size_t *num_entries,
                                         xjson_size_t *capacity)
{
    xjson_size_t new_num_entires = *num_entries + 1;
    while (new_num_entires >= *capacity) {
        *capacity = (*capacity + 1) * 1.7f;
        if ((base = realloc(base, *capacity * elem_size)) == NULL) {
            return xjson_result_realloc_err;
        }
    }
    return xjson_result_ok;
}

static xjson_result_e json_autoresize(xjson_json_t *object)
{
    return generic_autoresize(object->entries, sizeof(xjson_entry_t *), &object->num_entries, &object->capacity);
}

static xjson_result_e array_autoresize(xjson_array_t *array)
{
    return generic_autoresize(array->entries, sizeof(xjson_array_entry_t *), &array->num_entries, &array->capacity);
}

static void json_add_entry(xjson_json_t *object, xjson_entry_t *entry)
{
    object->entries[object->num_entries++] = entry;
}

static xjson_result_e json_add_complex(xjson_json_t **object, xjson_array_t **array, xjson_json_t *parent,
                                       const char *key, xjson_type_e complex_type, xjson_type_e array_type)
{
    xjson_json_t *retval_object;
    xjson_array_t *retval_array;
    xjson_entry_t *entry;
    xjson_result_e status;

    if (complex_type != xjson_array && complex_type != xjson_object) {
        return xjson_result_interalerr;
    } else {
        if (parent != NULL && key != NULL && ((entry = entry_create(parent, xjson_object, key)) != NULL) &&
            (((complex_type != xjson_object) || ((retval_object = json_create(parent->pool)) != NULL)) &&
             ((complex_type != xjson_array) || ((retval_array = array_create(entry, array_type)) != NULL)))) {
            if ((status = json_autoresize(parent)) == xjson_result_ok) {
                retval_object->parent = parent;
                json_add_entry(parent, entry);
                if (complex_type == xjson_object) {
                    entry->value->object = retval_object;
                    *object = retval_object;
                } else {
                    entry->value->array = retval_array;
                    *array = retval_array;
                }
                return xjson_result_ok;
            } return status;
        } else return xjson_result_nullptr;
    }
}

static xjson_value_t *value_create(xjson_pool_t *pool, xjson_entry_t *context, xjson_type_e type)
{
    xjson_value_t *value = pool_malloc_value(pool);
    value->type = type;
    value->context = context;
    return value;
}

static xjson_entry_t *entry_create(xjson_json_t *object, xjson_type_e type, const char *key)
{
    xjson_entry_t *entry = pool_malloc_entry(object->pool);
    xjson_value_t *value = value_create(object->pool, entry, type);
    entry->context = object;
    entry->key = pool_strdup(key, object->pool);
    entry->value = value;
    return entry;
}

static xjson_array_t *array_create(xjson_entry_t *context, xjson_type_e type)
{
    xjson_array_t *retval = pool_malloc_array(context->context->pool);
    retval->context = context;
    retval->num_entries = 0;
    retval->type = type;
    retval->capacity = XJSON_JSON_ARRAY_CAPACITY_DEFAULT;
    retval->entries = malloc(retval->capacity * sizeof(xjson_array_entry_t));
    return retval;
}

static xjson_json_t *pool_malloc_object(xjson_pool_t *pool)
{
    // TODO:...
    return NULL;
}

static xjson_array_t *pool_malloc_array(xjson_pool_t *pool)
{
    // TODO:...
    return NULL;
}

static xjson_entry_t *pool_malloc_entry(xjson_pool_t *pool)
{
    // TODO:...
    return NULL;
}

static xjson_value_t *pool_malloc_value(xjson_pool_t *pool)
{
    // TODO:...
    return NULL;
}

static xjson_array_entry_t *pool_malloc_array_entry(xjson_pool_t *pool)
{
    // TODO:...
    return NULL;
}

static char *pool_strdup(const char *str, xjson_pool_t *pool)
{
    // TODO:...
    return NULL;
}