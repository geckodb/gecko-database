#include <routers/api/1.0/collections.h>
#include <gs_collections.h>

void router_api_1_0_collections(gs_system_t *system, const gs_request_t *request, response_t *response)
{
    if (!gs_request_is_method(request, GS_GET)) {
        response_end(response, HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
    } else {
        gs_gridstore_t *gridstore = gs_system_get_gridstore(system);
        gs_collections_t *collections = gs_gridstore_get_collections(gridstore);
        char buffer[10240];
        FILE *memfile = fmemopen(buffer, ARRAY_LEN_OF(buffer), "w");
        gs_collections_print(memfile, collections);
        fclose(memfile);

        response_content_type_set(response, MIME_CONTENT_TYPE_APPLICATION_JSON);
        response_field_set(response, "Connection", "close");
        response_body_set(response, buffer);
        response_end(response, HTTP_STATUS_CODE_200_OK);
    }


}