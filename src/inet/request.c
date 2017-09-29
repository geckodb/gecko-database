// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
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

#include <inet/request.h>
#include <containers/dicts/hash_table.h>
#include <json-parser/json.h>

json_value *json_get(const json_value* value, const char *key, json_type type)
{
    if (value == NULL) {
        return NULL;
    } else {
        unsigned length = value->u.object.length;
        for (unsigned x = 0; x < length; x++) {
            json_object_entry entry = value->u.object.values[x];
            if (strcmp(entry.name, key) == 0) {
                return (entry.value->type == type) ? entry.value : NULL;
            }
        }
        return NULL;
    }
}

json_value* json_next(const json_value* value)
{
    static const json_value *it;
    static unsigned x;

    if (value == NULL) {
        return NULL;
    } else {
        it = value;
        x = 0;
    }
    if (it->type != json_array) {
        return NULL;
    }

    if (x >= it->u.array.length) {
        return NULL;
    } else {
        return it->u.array.values[x++];
    }
}

static void print_depth_shift(int depth)
{
    int j;
    for (j=0; j < depth; j++) {
        printf(" ");
    }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.object.length;
    for (x = 0; x < length; x++) {
        print_depth_shift(depth);
        printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
        process_value(value->u.object.values[x].value, depth+1);
    }
}

static void process_array(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.array.length;
    printf("array\n");
    for (x = 0; x < length; x++) {
        process_value(value->u.array.values[x], depth);
    }
}

static void process_value(json_value* value, int depth)
{
    if (value == NULL) {
        return;
    }
    if (value->type != json_object) {
        print_depth_shift(depth);
    }
    switch (value->type) {
        case json_null:
            printf("null\n");
            break;
        case json_none:
            printf("none\n");
            break;
        case json_object:
            process_object(value, depth+1);
            break;
        case json_array:
            process_array(value, depth+1);
            break;
        case json_integer:
            printf("int: %10" PRId64 "\n", value->u.integer);
            break;
        case json_double:
            printf("double: %f\n", value->u.dbl);
            break;
        case json_string:
            printf("string: %s\n", value->u.string.ptr);
            break;
        case json_boolean:
            printf("bool: %d\n", value->u.boolean);
            break;
    }
}

static inline bool parse_params(dict_t *dict, const char *string)
{
    json_char* json;
    json_value* value;

    char *json_doc = strstr(string, "=");
    if (json_doc && strlen(json_doc) > 1) {
        json = (json_char*) json_doc + 1;
        printf("%s\n", json);

        if ((value = json_parse(json, strlen(string))) == NULL) {
            fprintf(stderr, "Unable to parse data\n");
            return false;
        } else {
           // process_value(value, 0);

            const json_value *value1 = json_get(value, "name", json_string);
            const json_value *value2 = json_get(value, "name", json_array);
            printf ("XXX %p, %p\n", value1, value2);
            const json_value *value3 = json_get(value, "schema", json_array);
            const json_value *it = json_next(value3);
            while (it != NULL) {
                const json_value *primary = json_get(it, "primary", json_string);
                if (primary != NULL) {
                    printf("\n>>> PRIMAR? %s\n", primary->u.string.ptr);
                }

                it = json_next(NULL);
            }

            json_value_free(value);
            return true;
        }
    } else return false;

}

static inline bool strstart(const char *string, const char *substr)
{
    return (strncmp(string, substr, strlen(substr)) == 0);
}

void request_parse(request_t *request, const char *string) {
    GS_REQUIRE_NONNULL(request)
    char *request_str = strdup(string);
    request_str[strstr(request_str, "\n") - request_str] = '\0';
    request->params = hash_table_new_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                        sizeof(char *), sizeof(char *), 15,
                                        15, 1.7f, 0.75f,
                                        str_equals, str_str_clean_up, true);

    char *method_str = NULL;
    if (strstart(request_str, HTTP_GET_STRING)) {
        request->method = HTTP_GET;
        method_str = HTTP_GET_STRING;
    } else if (strstart(request_str, HTTP_PUT_STRING)) {
        request->method = HTTP_PUT;
        method_str = HTTP_PUT_STRING;
    } else if (strstart(request_str, HTTP_POST_STRING)) {
        request->method = HTTP_POST;
        method_str = HTTP_POST_STRING;
    } else if (strstart(request_str, HTTP_DELETE_STRING)) {
        request->method = HTTP_DELETE;
        method_str = HTTP_DELETE_STRING;
    } else {
        request->method = HTTP_OTHERS;
        request->valid = false;
        return;
    }

    if (strlen(request_str) - strlen(method_str) > 0) {
        char *original = strdup(request_str + strlen(method_str) + 1);
        char *http_resource = strdup(original);

        if (strstr(request_str, "?") != NULL) {
            char *tail = strstr(http_resource, "?");
            http_resource[strlen(http_resource) - strlen(tail)] = '\0';
            request->resource = http_resource;
            char *params = strdup(original + strlen(http_resource) + 1);
            params[strlen(params) - strlen(strstr(params, "HTTP/1")) - 1] = '\0';
            parse_params(request->params, params);
        } else {
            request->resource = http_resource;
            request->resource[strlen(request->resource) - strlen(strstr(request->resource, "HTTP/1")) - 1] = '\0';
            request->params = NULL;
        }

        request->valid = true;
    } else {
        request->valid = false;
    }
}

void request_print(FILE *file, const request_t *request)
{
    GS_REQUIRE_NONNULL(file)
    GS_REQUIRE_NONNULL(request)
    printf("request{method=%s, is_valid='%d', resource='%s', params='%s'}\n",
           methodstr(request->method), request->valid, request->resource, "...");
}