#pragma once

#include <stdinc.h>
#include <inet/request.h>
#include <inet/response.h>

static inline void router_api_types_create(capture_t *capture, const request_t *request, response_t *response)
{

    response_content_type_set(response, "text/html");//MIME_CONTENT_TYPE_TEXT_PLAIN);
    response_field_set(response, "Connection", "close");
    response_field_set(response, "Content-Length", "40");
    response_body_set(response, "<html><body>Hello routers</body></html>\n");
    response_end(response, HTTP_STATUS_CODE_200_OK);
}