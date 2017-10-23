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

#include <xjson/xjson.h>

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct xjson_json_t
{
    xjson_json_t            *parent;
    xjson_entry_t           *entries;
    xjson_size_t             num_entries;
} xjson_json_t;

typedef struct xjson_array_t
{
    xjson_entry_t           *context;
    xjson_size_t             num_entries;
    xjson_size_t             capacity;
    xjson_type_e             type;
    xjson_array_entry_t     *entries;
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
        xjson_array_t        array;
        xjson_json_t         object;
        xjson_type_integer_t integer;
        xjson_type_double_t  decimal;
        xjson_type_string_t  string;
    };
} xjson_value_t;

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

xjson_result_e  xjson_json_create(xjson_json_t **json)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_json_parse(xjson_json_t **json, const char *text)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_json_dispose(xjson_json_t *json)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_json_print(FILE *file, const xjson_json_t *json)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_json_add(xjson_json_t *json, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_json_remove(xjson_json_t *json, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_json_query(xjson_json_cursor_t **cursor, const xjson_json_t *json,
                                 const void *capture, xjson_entry_pred_t pred)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_next(xjson_json_cursor_t *cursor)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_rewind(xjson_json_cursor_t *cursor)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_close(xjson_json_cursor_t *cursor)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_length(xjson_size_t **length, const xjson_json_cursor_t *cursor)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_get_type(xjson_type_e *type, xjson_json_cursor_t *cursor)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_get_context(xjson_json_t **context, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_get_key(const char **key, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_is_null(const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_is_none(const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_has_value(const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_get_index(xjson_size_t **idx, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_get_object(xjson_json_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_get_array(xjson_array_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_get_number_int(xjson_type_integer_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_get_number_double(xjson_type_double_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_get_number_string(const char **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_query_value_get_number_boolean(xjson_bool_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_create_object(xjson_entry_t **entry, const char *key, const xjson_json_t *value)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_create_array(xjson_entry_t **entry, xjson_type_e type)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_create_number_int(xjson_entry_t **entry, const char *key, xjson_type_integer_t value)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_create_number_double(xjson_entry_t **entry, const char *key, xjson_type_double_t value)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_create_string(xjson_entry_t **entry, const char *key, const char *value)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_create_boolean(xjson_entry_t **entry, const char *key, xjson_bool_t value)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_create_null(xjson_entry_t **entry, const char *key)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_dispose(xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_get_type(xjson_type_e *type, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_get_context(xjson_json_t **context, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_get_key(const char **key, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_is_null(const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_is_none(const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_has_value(const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_value_get_object(xjson_json_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_value_get_array(xjson_array_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_value_get_number_int(xjson_type_integer_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_value_get_number_double(xjson_type_double_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_value_get_number_string(const char **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_entry_value_get_number_boolean(xjson_bool_t **value, const xjson_entry_t *entry)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_array_get_length(xjson_size_t **length, const xjson_array_t *array)
{
    return xjson_result_ok;
}

xjson_result_e  xjson_array_query(xjson_array_t *array, const void *capture, xjson_array_pred_t pred)
{
    return xjson_result_ok;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------