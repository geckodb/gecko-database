#pragma once

#include <gs.h>
#include <inet/gs_response.h>

 void router_catch(gs_system_t *system, const gs_request_t *request, gs_response_t *response)
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