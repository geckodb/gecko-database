#pragma once

#include <gs.h>

typedef enum gs_request_body_e gs_request_body_e;

typedef struct gs_request_t gs_request_t;

typedef enum gs_http_method_e {
    GS_OPTIONS, GS_HEAD, GS_TRACE, GS_POST, GS_GET, GS_PUT, GS_DELETE, GS_CONNECT, GS_UNKNOWN
} gs_http_method_e;

__BEGIN_DECLS

GS_DECLARE(gs_status_t) gs_request_create(gs_request_t **request, int socket_desc);

GS_DECLARE(gs_status_t) gs_request_dispose(gs_request_t **request);

GS_DECLARE(gs_status_t) gs_request_raw(char **original, const gs_request_t *request);

GS_DECLARE(gs_status_t) gs_request_has_field(const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_field_by_name(char const** value, const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_has_form(const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_form_by_name(char const** value, const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_method(gs_http_method_e *method, const gs_request_t *request);

GS_DECLARE(gs_status_t) gs_request_resource(char **resource, const gs_request_t *request);

GS_DECLARE(gs_status_t) gs_request_is_valid(const gs_request_t *request);

__END_DECLS