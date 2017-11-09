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
#include <gs_http.h>
#include <apr_hash.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define MIME_CONTENT_TYPE                   "Content-Type"
#define MIME_CONTENT_TYPE_TEXT_PLAIN        "text/plain"
#define MIME_CONTENT_TYPE_APPLICATION_JSON  "application/json"

#define RESPONSE_DICT_CAPACITY              10

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_response_t {
    gs_http_status_code_t code;
    apr_hash_t *fields;
    char *body;
    apr_pool_t *pool;
} gs_response_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void gs_response_create(gs_response_t *response);
void gs_response_dispose(gs_response_t *response);
char *gs_response_pack(gs_response_t *response);
void gs_response_field_set(gs_response_t *response, const char *field, const char *value);
const char *gs_response_field_get(gs_response_t *response, const char *field);
void gs_response_body_set(gs_response_t *response, const char *body);
void gs_response_content_type_set(gs_response_t *response, const char *content_type);
const char *gs_response_content_type_get(gs_response_t *response);
const char *gs_response_body_get(gs_response_t *response);
void gs_response_end(gs_response_t *response, gs_http_status_code_t code);
const char *gs_response_format_fields(const gs_response_t *response);