#pragma once

#include <stdinc.h>

typedef uint32_t http_status_code_t;

#define HTTP_STATUS_CODE_200_OK             200
#define HTTP_STATUS_CODE_500_INTERNAL_ERR   500

#define MIME_CONTENT_TYPE                   "Content-Type"
#define MIME_CONTENT_TYPE_TEXT_PLAIN        "text/plain"

#define RESPONSE_DICT_CAPACITY  10

typedef struct response_t {
    http_status_code_t code;
    dict_t *fields;
    char *body;
} response_t;

void gs_response_create(response_t *response);
char *gs_response_pack(response_t *response);
void gs_response_free(response_t *response);
void gs_response_field_set(response_t *response, const char *field, const char *value);
const char *gs_response_field_get(response_t *response, const char *field);
void gs_response_body_set(response_t *response, const char *body);
void gs_response_content_type_set(response_t *response, const char *content_type);
const char *gs_response_content_type_get(response_t *response);
const char *gs_response_body_get(response_t *response);
void gs_response_end(response_t *response, http_status_code_t code);

const struct vector_t *gs_response_fields(response_t *response);
const char *gs_response_code_str(http_status_code_t code);
const char *gs_response_format_fields(const dict_t *fields);