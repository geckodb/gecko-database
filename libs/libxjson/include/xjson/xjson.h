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

#ifndef LIBXJSON_H
#define LIBXJSON_H

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define XJSON_ROOT                   NULL

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#ifndef XJSON_POOL_OBJ_CAPACITY_DEFAULT
    #define XJSON_POOL_OBJ_CAPACITY_DEFAULT     15
#endif
#ifndef XJSON_POOL_ARRAY_CAPACITY_DEFAULT
    #define XJSON_POOL_ARRAY_CAPACITY_DEFAULT   15
#endif
#ifndef XJSON_JSON_ENTRIES_CAPACITY_DEFAULT
#define XJSON_JSON_ENTRIES_CAPACITY_DEFAULT     15
#endif
#ifndef XJSON_JSON_ARRAY_CAPACITY_DEFAULT
#define XJSON_JSON_ARRAY_CAPACITY_DEFAULT       15
#endif

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum
{
    xjson_none,
    xjson_object,
    xjson_array,

    xjson_number_integer,
    xjson_number_double,
    xjson_string,
    xjson_boolean,

    xjson_null
}                                   xjson_type_e;

typedef enum
{
    xjson_result_ok,
    xjson_result_failed,
    xjson_result_malloc_err,
    xjson_result_pmalloc_err,
    xjson_result_realloc_err,
    xjson_result_nullptr,
    xjson_result_notype,
    xjson_result_interalerr,
    xjson_result_wrongusage,
    xjson_result_nopool
}                                   xjson_result_e;

typedef unsigned long long          xjson_size_t;

typedef long long                   xjson_u64_t;

typedef double                      xjson_double_t;

typedef int                         xjson_bool_t;

typedef xjson_u64_t                 xjson_type_integer_t;

typedef xjson_double_t              xjson_type_double_t;

typedef char *                      xjson_type_string_t;

typedef xjson_bool_t                xjson_type_boolean_t;

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct xjson_pool_t         xjson_pool_t;

typedef struct xjson_json_t         xjson_json_t;

typedef struct xjson_array_t        xjson_array_t;

typedef struct xjson_array_entry_t  xjson_array_entry_t;

typedef struct xjson_entry_t        xjson_entry_t;

typedef struct xjson_value_t        xjson_value_t;

typedef struct xjson_json_cursor_t  xjson_json_cursor_t;

// ---------------------------------------------------------------------------------------------------------------------
// F U N C T I O N   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

typedef void (*xjson_entry_pred_t)(xjson_bool_t *result, const void *capture,
                                   const xjson_entry_t *begin, const xjson_entry_t *end);

typedef void (*xjson_array_pred_t)(xjson_bool_t *result, const void *capture,
                                   const xjson_array_entry_t *begin, const xjson_array_entry_t *end);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

xjson_result_e  xjson_pool_create(xjson_pool_t **pool);

xjson_result_e  xjson_pool_dispose(xjson_pool_t **pool);

xjson_result_e  xjson_json_create(xjson_json_t **json, xjson_pool_t *pool);

xjson_result_e  xjson_json_parse(xjson_json_t **json, const char *text);

xjson_result_e  xjson_json_print(FILE *file, const xjson_json_t *json);

xjson_result_e  xjson_json_add_property(xjson_json_t *parent, xjson_type_e type, const char *key, const void *data);

xjson_result_e  xjson_json_add_object(xjson_json_t **object, xjson_json_t *parent, const char *key);

xjson_result_e  xjson_json_add_array(xjson_array_t **array, xjson_json_t *parent, xjson_type_e type, const char *key);

xjson_result_e  xjson_array_add_property(xjson_array_t *parent, const void *data);

xjson_result_e  xjson_array_add_object(xjson_json_t **object, const xjson_array_t *parent);

xjson_result_e  xjson_array_add_array(xjson_entry_t **array, const xjson_array_t *parent);



/*


xjson_result_e  xjson_json_remove(xjson_json_t *json, const xjson_entry_t *entry);

xjson_result_e  xjson_json_query(xjson_json_cursor_t **cursor, const xjson_json_t *json,
                                 const void *capture, xjson_entry_pred_t pred);

xjson_result_e  xjson_query_next(xjson_json_cursor_t *cursor);

xjson_result_e  xjson_query_rewind(xjson_json_cursor_t *cursor);

xjson_result_e  xjson_query_close(xjson_json_cursor_t *cursor);

xjson_result_e  xjson_query_length(xjson_size_t **length, const xjson_json_cursor_t *cursor);

xjson_result_e  xjson_query_value_get_type(xjson_type_e *type, xjson_json_cursor_t *cursor);

xjson_result_e  xjson_query_get_context(xjson_json_t **context, const xjson_entry_t *entry);

xjson_result_e  xjson_query_get_key(const char **key, const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_is_null(const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_is_none(const xjson_entry_t *entry);

xjson_result_e  xjson_query_has_value(const xjson_entry_t *entry);

xjson_result_e  xjson_query_get_index(xjson_size_t **idx, const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_get_object(xjson_json_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_get_array(xjson_array_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_get_number_int(xjson_type_integer_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_get_number_double(xjson_type_double_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_get_number_string(const char **value, const xjson_entry_t *entry);

xjson_result_e  xjson_query_value_get_number_boolean(xjson_bool_t **value, const xjson_entry_t *entry);



xjson_result_e  xjson_entry_get_type(xjson_type_e *type, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_get_context(xjson_json_t **context, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_get_key(const char **key, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_is_null(const xjson_entry_t *entry);

xjson_result_e  xjson_entry_is_none(const xjson_entry_t *entry);

xjson_result_e  xjson_entry_has_value(const xjson_entry_t *entry);

xjson_result_e  xjson_entry_value_get_object(xjson_json_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_value_get_array(xjson_array_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_value_get_number_int(xjson_type_integer_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_value_get_number_double(xjson_type_double_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_value_get_number_string(const char **value, const xjson_entry_t *entry);

xjson_result_e  xjson_entry_value_get_number_boolean(xjson_bool_t **value, const xjson_entry_t *entry);

xjson_result_e  xjson_array_get_length(xjson_size_t **length, const xjson_array_t *array);



xjson_result_e  xjson_array_query(xjson_array_t *array, const void *capture, xjson_array_pred_t pred);*/

#ifdef __cplusplus
}
#endif

#endif // LIBXJSON_H
