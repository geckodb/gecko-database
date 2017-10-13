#include <routers/api/1.0/nodes.h>

void router_api_1_0_nodes(gs_system_t *system, const gs_request_t *request, response_t *response)
{
    if (!gs_request_is_method(request, GS_POST)) {
        response_end(response, HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
    } else {


       // gs_dispatcher_waitfor(dispatcher, gs_event_gridstore_invoke());


        response_content_type_set(response, "text/html");//MIME_CONTENT_TYPE_TEXT_PLAIN);
        response_field_set(response, "Connection", "close");
        response_body_set(response, "<html><body>Hello routers</body></html>\n");
        response_end(response, HTTP_STATUS_CODE_200_OK);
    }


}