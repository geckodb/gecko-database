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

typedef enum method_t {
    HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE, HTTP_OTHERS
} method_t;

typedef struct request_t {
    method_t method;
    char *resource;
    char *params;
    bool valid;
} request_t;

void request_parse(request_t *request, const char *request_str);

void request_print(FILE *file, const request_t *request);
