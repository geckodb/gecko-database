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
// If not, see .

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <routers/api/1.0/gs_nodes.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_router_api_1_0_nodes(gs_dispatcher_t *dispatcher, const gs_request_t *request, gs_response_t *response)
{
    if (!gs_request_is_method(request, GS_POST)) {
        gs_response_end(response, HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
    } else {


        gs_dispatcher_waitfor(dispatcher, gs_event_gridstore_invoke());


        gs_response_content_type_set(response, "text/html");//MIME_CONTENT_TYPE_TEXT_PLAIN);
        gs_response_field_set(response, "Connection", "close");
        gs_response_field_set(response, "Content-Length", "40");
        gs_response_body_set(response, "<html><body>Hello routers</body></html>\n");
        gs_response_end(response, HTTP_STATUS_CODE_200_OK);
    }


}