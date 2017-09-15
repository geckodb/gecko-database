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

#define MIME_CONTENT_TYPE                   "Content-Type"
#define MIME_CONTENT_TYPE_TEXT_PLAIN        "text/plain"

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define RESPONSE_DICT_CAPACITY  10

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct response_t {
    http_status_code_t code;
    dict_t *fields;
    char *body;
} response_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void response_create(response_t *response);
void response_dispose(response_t *response);
char *response_pack(response_t *response);
void response_field_set(response_t *response, const char *field, const char *value);
const char *response_field_get(response_t *response, const char *field);
void response_body_set(response_t *response, const char *body);
void response_content_type_set(response_t *response, const char *content_type);
const char *response_content_type_get(response_t *response);
const char *response_body_get(response_t *response);
void response_end(response_t *response, http_status_code_t code);
const struct vec_t *response_fields(response_t *response);
const char *response_format_fields(const dict_t *fields);