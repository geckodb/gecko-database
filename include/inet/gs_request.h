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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum gs_request_body_e gs_request_body_e;

typedef struct gs_request_t gs_request_t;

typedef enum gs_http_method_e {
    GS_OPTIONS, GS_HEAD, GS_TRACE, GS_POST, GS_GET, GS_PUT, GS_DELETE, GS_CONNECT, GS_UNKNOWN
} gs_http_method_e;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

GS_DECLARE(gs_status_t) gs_request_create(gs_request_t **request, int socket_desc);

GS_DECLARE(gs_status_t) gs_request_dispose(gs_request_t **request);

GS_DECLARE(gs_status_t) gs_request_raw(char **original, const gs_request_t *request);

GS_DECLARE(gs_status_t) gs_request_has_field(const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_field_by_name(char const** value, const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_has_form(const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_form_by_name(char const** value, const gs_request_t *request, const char *key);

GS_DECLARE(gs_status_t) gs_request_method(gs_http_method_e *method, const gs_request_t *request);

GS_DECLARE(gs_status_t) gs_request_is_method(const gs_request_t *request, gs_http_method_e method);

GS_DECLARE(gs_status_t) gs_request_resource(char **resource, const gs_request_t *request);

GS_DECLARE(gs_status_t) gs_request_is_valid(const gs_request_t *request);

__END_DECLS