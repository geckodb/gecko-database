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

#include <inet/gs_response.h>
#include <apr_strings.h>

void response_create(response_t *response)
{
    GS_REQUIRE_NONNULL(response);
    response->code = HTTP_STATUS_CODE_500_INTERNAL_ERR;
    response->body = NULL;
    apr_pool_create(&response->pool, NULL);
    response->fields = apr_hash_make(response->pool);
}

char *response_pack(response_t *response)
{
    GS_REQUIRE_NONNULL(response);
    const char *pack = "HTTP/1.1 %s\r\n%s\r\n%s";
    const char *code = codestr(response->code);
    const char *mime = response_format_fields(response);
    const char *body = response_body_get(response);
    char *buffer = GS_REQUIRE_MALLOC(strlen(pack) + 1 + strlen(mime) + 1 + strlen(body) + 1);
    sprintf(buffer, pack, code, mime, body);
    return buffer;
}

void response_field_set(response_t *response, const char *field, const char *value)
{
    GS_REQUIRE_NONNULL(response);
    GS_REQUIRE_NONNULL(response->fields);
    char *imp_key = apr_pstrdup(response->pool, field);
    char *imp_val = apr_pstrdup(response->pool, value);
    apr_hash_set(response->fields, imp_key, APR_HASH_KEY_STRING, imp_val);
}

const char *response_field_get(response_t *response, const char *field)
{
    GS_REQUIRE_NONNULL(response);
    GS_REQUIRE_NONNULL(field);
    GS_REQUIRE_NONNULL(response->fields);
    const void *result = apr_hash_get(response->fields, field, APR_HASH_KEY_STRING);
    return (result != NULL ? *(char **) result : "");
}

void response_body_set(response_t *response, const char *body)
{
    GS_REQUIRE_NONNULL(response);
    GS_REQUIRE_NONNULL(body);
    if (response->body != NULL) {
        free (response->body);
    } else {
        response->body = GS_REQUIRE_MALLOC(strlen(body) + 1);
        strcpy(response->body, body);
    }
}

void response_content_type_set(response_t *response, const char *content_type)
{
    GS_REQUIRE_NONNULL(response);
    GS_REQUIRE_NONNULL(response->fields);
    GS_REQUIRE_NONNULL(content_type);
    response_field_set(response, MIME_CONTENT_TYPE, content_type);
}

const char *response_content_type_get(response_t *response)
{
    return response_field_get(response, MIME_CONTENT_TYPE);
}

const char *response_body_get(response_t *response)
{
    GS_REQUIRE_NONNULL(response);
    return (response->body != NULL ? response->body : "");
}

void response_dispose(response_t *response)
{
    GS_REQUIRE_NONNULL(response);
    GS_REQUIRE_NONNULL(response->fields);
    if (response->body != NULL) {
        apr_pool_destroy(response->pool);
        free (response->body);
    }
   // dict_delete(response->fields);
}

void response_end(response_t *response, http_status_code_t code)
{
    GS_REQUIRE_NONNULL(response);
    response->code = code;
}

const char *response_format_fields(const response_t *response)
{
    GS_REQUIRE_NONNULL(response);

    size_t length_fields = 0;

    for (apr_hash_index_t *it = apr_hash_first(response->pool, response->fields); it; it = apr_hash_next(it)) {
        const char *field_name, *value;
        apr_hash_this(it, (const void **) &field_name, NULL, (void **) &value);
        const char *value_str = (value != NULL ? value : "");
        length_fields += strlen(field_name) + 2 + strlen(value_str) + 2;    // "str: " + "value_str\r\n";
    }
    length_fields += 2; // null terminator

    char *formatted_str = GS_REQUIRE_MALLOC(length_fields);
    size_t offset = 0;
    for (apr_hash_index_t *it = apr_hash_first(response->pool, response->fields); it; it = apr_hash_next(it)) {
        const char *field_name, *value;
        apr_hash_this(it, (const void **) &field_name, NULL, (void **) &value);
        const char *value_str = (value != NULL ? value : "");

        strcpy(formatted_str + offset, field_name);
        offset += strlen(field_name);
        strcpy(formatted_str + offset, ": ");
        offset += 2;
        strcpy(formatted_str + offset, value_str);
        offset += strlen(value_str);
        strcpy(formatted_str + offset, "\r\n");
        offset += 2;
    }
    strcpy(formatted_str + offset, "\0");
    return formatted_str;
}