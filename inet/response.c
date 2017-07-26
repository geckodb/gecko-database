#include <inet/response.h>
#include <containers/dicts/hash_table.h>

void gs_response_create(response_t *response)
{
    require_non_null(response);
    response->code = HTTP_STATUS_CODE_500_INTERNAL_ERR;
    response->body = NULL;
    response->fields = hash_table_create_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                           sizeof(char *), sizeof(char *), RESPONSE_DICT_CAPACITY,
                                           RESPONSE_DICT_CAPACITY, 1.7f, 0.75f,
                                           str_equals, clean_up, true);
}

char *gs_response_pack(response_t *response)
{
    require_non_null(response);
    const char *pack = "HTTP/1.1 %s\r\n%s\r\n\r\n%s";
    const char *code = gs_response_code_str(response->code);
    const char *mime = gs_response_format_fields(response->fields);
    const char *body = gs_response_body_get(response);
    char *buffer = require_good_malloc(strlen(pack) + 1 + strlen(mime) + 1 + strlen(body) + 1);
    sprintf(buffer, pack, code, mime, body);
    return buffer;
}

void gs_response_field_set(response_t *response, const char *field, const char *value)
{
    require_non_null(response);
    require_non_null(response->fields);
    const char *imp_key = strdup(field);
    const char *imp_val = strdup(value);
    dict_put(response->fields, &imp_key, &imp_val);
}

const char *gs_response_field_get(response_t *response, const char *field)
{
    require_non_null(response);
    require_non_null(field);
    require_non_null(response->fields);
    const void *result = dict_get(response->fields, field);
    return (result != NULL ? *(char **) result : "");
}

void gs_response_body_set(response_t *response, const char *body)
{
    require_non_null(response);
    require_non_null(body);
    if (response->body != NULL) {
        free (response->body);
    } else {
        response->body = malloc(strlen(body) + 1);
        strcpy(response->body, body);
    }
}

void gs_response_content_type_set(response_t *response, const char *content_type)
{
    require_non_null(response);
    require_non_null(response->fields);
    require_non_null(content_type);
    gs_response_field_set(response, MIME_CONTENT_TYPE, content_type);
}

const char *gs_response_content_type_get(response_t *response)
{
    return gs_response_field_get(response, MIME_CONTENT_TYPE);
}

const char *gs_response_body_get(response_t *response)
{
    require_non_null(response);
    return (response->body != NULL ? response->body : "");
}

void gs_response_free(response_t *response)
{
    require_non_null(response);
    require_non_null(response->fields);
    if (response->body != NULL) {
        free (response->body);
    }
   // dict_free(response->fields);
}

void gs_response_end(response_t *response, http_status_code_t code)
{
    require_non_null(response);
    response->code = code;
}

const struct vector_t *gs_response_fields(response_t *response)
{
    require_non_null(response);
    return dict_keyset(response->fields);
}

const char *gs_response_code_str(http_status_code_t code)
{
    switch (code) {
        case HTTP_STATUS_CODE_200_OK:               return "200 OK";
        case HTTP_STATUS_CODE_500_INTERNAL_ERR:
        default:                                    return "500 Internal Server Error";
    }
}

const char *gs_response_format_fields(const dict_t *fields)
{
    require_non_null(fields);

    const struct vector_t *keys = dict_keyset(fields);
    size_t length_fields = 0;
    size_t num_elements = vector_num_elements(keys);
    for (size_t i = 0; i < num_elements; i++) {
        const char *field_name = (const char *) vector_at(keys, i);
        const void *value = dict_get(fields, field_name);
        const char *value_str = (value != NULL ? (const char *) value : "");
        length_fields += strlen(field_name) + 2 + strlen(value_str) + 2;    // "str: " + "value_str\r\n";
    }
    length_fields += 1; // null terminator

    char *formatted_str = require_good_malloc(length_fields);
    size_t offset = 0;
    for (size_t i = 0; i < num_elements; i++) {
        const char *field_name = (const char *) vector_at(keys, i);
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