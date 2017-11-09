// Several pre-defined functions related to hash operations
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
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define HTTP_STATUS_CODE_100_CONTINUE           100
#define HTTP_STATUS_CODE_200_OK                 200
#define HTTP_STATUS_CODE_500_INTERNAL_ERR       500
#define HTTP_STATUS_CODE_400_BAD_REQUEST        400
#define HTTP_STATUS_CODE_404_NOT_FOUND          404
#define HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED 405
#define HTTP_STATUS_CODE_408_REQUEST_TIMEOUT    408


#define HTTP_GET_STRING    "GET"
#define HTTP_PUT_STRING    "PUT"
#define HTTP_POST_STRING   "POST"
#define HTTP_DELETE_STRING "DELETE"

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef uint32_t gs_http_status_code_t;

typedef enum gs_content_type_e
{
    MEDIA_MULTI_PART_FORM_DATA
} gs_content_type_e;

typedef enum gs_method_e {
    HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE, HTTP_OTHERS
} gs_method_e;


// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

const char *gs_codestr(gs_http_status_code_t code);

const char *gs_methodstr(gs_http_status_code_t code);