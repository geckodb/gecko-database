#pragma once

#include <gs.h>
#include <inet/response.h>
#include <inet/gs_request.h>

static inline void router_api_1_0_nodes(capture_t *capture, const gs_request_t *request, response_t *response)
{
    if (!gs_request_is_method(request, GS_POST)) {
        response_end(response, HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
    } else {
        response_content_type_set(response, "text/html");//MIME_CONTENT_TYPE_TEXT_PLAIN);
        response_field_set(response, "Connection", "close");
        response_field_set(response, "Content-Length", "40");
        response_body_set(response, "<html><body>Hello routers</body></html>\n");
        response_end(response, HTTP_STATUS_CODE_200_OK);
    }


}