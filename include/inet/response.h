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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdinc.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define HTTP_STATUS_CODE_200_OK             200
#define HTTP_STATUS_CODE_500_INTERNAL_ERR   500

#define MIME_CONTENT_TYPE                   "Content-Type"
#define MIME_CONTENT_TYPE_TEXT_PLAIN        "text/plain"

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define RESPONSE_DICT_CAPACITY  10

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef uint32_t http_status_code_t;

typedef struct response_t {
    http_status_code_t code;
    dict_t *fields;
    char *body;
} response_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

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