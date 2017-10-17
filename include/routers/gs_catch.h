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
#include <inet/gs_response.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

 void gs_router_catch(gs_dispatcher_t *dispatcher, const gs_request_t *request, gs_response_t *response)
{
    char buffer[1024];
    char *resource;
    gs_request_resource(&resource, request);
    gs_response_content_type_set(response, MIME_CONTENT_TYPE_APPLICATION_JSON);
    sprintf(buffer, "{\r\n\t\""
                        "code\" : \"404\",\r\n\t\""
                        "message\" : \"resource not found\",\r\n\t\""
                        "resource\" : \"%s\"\r\n}\n", resource);

    gs_response_body_set(response, buffer);
    gs_response_end(response, HTTP_STATUS_CODE_404_NOT_FOUND);
}