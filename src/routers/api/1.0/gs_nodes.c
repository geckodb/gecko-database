#include <routers/api/1.0/gs_nodes.h>

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