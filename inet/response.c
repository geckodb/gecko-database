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

#include <inet/response.h>
#include <containers/dicts/hash_table.h>

void response_create(response_t *response)
{
    REQUIRE_NONNULL(response);
    response->code = HTTP_STATUS_CODE_500_INTERNAL_ERR;
    response->body = NULL;
    response->fields = hash_table_new_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                         sizeof(char *), sizeof(char *), RESPONSE_DICT_CAPACITY,
                                         RESPONSE_DICT_CAPACITY, 1.7f, 0.75f,
                                         str_equals, clean_up, true);
}

char *response_pack(response_t *response)
{
    REQUIRE_NONNULL(response);
    const char *pack = "HTTP/1.1 %s\r\n%s\r\n\r\n%s";
    const char *code = response_code_str(response->code);
    const char *mime = response_format_fields(response->fields);
    const char *body = response_body_get(response);
    char *buffer = REQUIRE_MALLOC(strlen(pack) + 1 + strlen(mime) + 1 + strlen(body) + 1);
    sprintf(buffer, pack, code, mime, body);
    return buffer;
}

void response_field_set(response_t *response, const char *field, const char *value)
{
    REQUIRE_NONNULL(response);
    REQUIRE_NONNULL(response->fields);
    const char *imp_key = strdup(field);
    const char *imp_val = strdup(value);
    dict_put(response->fields, &imp_key, &imp_val);
}

const char *response_field_get(response_t *response, const char *field)
{
    REQUIRE_NONNULL(response);
    REQUIRE_NONNULL(field);
    REQUIRE_NONNULL(response->fields);
    const void *result = dict_get(response->fields, field);
    return (result != NULL ? *(char **) result : "");
}

void response_body_set(response_t *response, const char *body)
{
    REQUIRE_NONNULL(response);
    REQUIRE_NONNULL(body);
    if (response->body != NULL) {
        free (response->body);
    } else {
        response->body = REQUIRE_MALLOC(strlen(body) + 1);
        strcpy(response->body, body);
    }
}

void response_content_type_set(response_t *response, const char *content_type)
{
    REQUIRE_NONNULL(response);
    REQUIRE_NONNULL(response->fields);
    REQUIRE_NONNULL(content_type);
    response_field_set(response, MIME_CONTENT_TYPE, content_type);
}

const char *response_content_type_get(response_t *response)
{
    return response_field_get(response, MIME_CONTENT_TYPE);
}

const char *response_body_get(response_t *response)
{
    REQUIRE_NONNULL(response);
    return (response->body != NULL ? response->body : "");
}

void response_dispose(response_t *response)
{
    REQUIRE_NONNULL(response);
    REQUIRE_NONNULL(response->fields);
    if (response->body != NULL) {
        free (response->body);
    }
   // dict_delete(response->fields);
}

void response_end(response_t *response, http_status_code_t code)
{
    REQUIRE_NONNULL(response);
    response->code = code;
}

const struct vec_t *response_fields(response_t *response)
{
    REQUIRE_NONNULL(response);
    return dict_keyset(response->fields);
}

const char *response_code_str(http_status_code_t code)
{
    switch (code) {
        case HTTP_STATUS_CODE_200_OK:               return "200 OK";
        case HTTP_STATUS_CODE_500_INTERNAL_ERR:
        default:                                    return "500 Internal Server Error";
    }
}

const char *response_format_fields(const dict_t *fields)
{
    REQUIRE_NONNULL(fields);

    const struct vec_t *keys = dict_keyset(fields);
    size_t length_fields = 0;
    size_t num_elements = vec_length(keys);
    for (size_t i = 0; i < num_elements; i++) {
        const char *field_name = (const char *) vec_at(keys, i);
        const void *value = dict_get(fields, field_name);
        const char *value_str = (value != NULL ? (const char *) value : "");
        length_fields += strlen(field_name) + 2 + strlen(value_str) + 2;    // "str: " + "value_str\r\n";
    }
    length_fields += 1; // null terminator

    char *formatted_str = REQUIRE_MALLOC(length_fields);
    size_t offset = 0;
    for (size_t i = 0; i < num_elements; i++) {
        const char *field_name = (const char *) vec_at(keys, i);
        const void *value = dict_get(fields, field_name);
        const char *value_str = (value != NULL ? (const char *) value : "");
        memcpy(formatted_str + offset, field_name, strlen(field_name));
        offset += strlen(field_name);
        memcpy(formatted_str + offset, ": ", 2);
        offset += 2;
        memcpy(formatted_str + offset, value_str, strlen(value_str));
        offset += strlen(value_str);
        memcpy(formatted_str + offset, "\r\n", 2);
    }
    memcpy(formatted_str + offset, "\r\n", 2);
    return formatted_str;
}