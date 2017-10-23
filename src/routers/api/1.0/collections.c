#include <routers/api/1.0/collections.h>
#include <gs_collections.h>
#include <json-parser/json.h>

#define API_COLLECTIONS_CREATE_COLLECTION_KEY "collection"

static void process_list_collections(char *buffer, size_t buffer_length, gs_collections_t *collections, response_t *response);
static void process_create_collections(char *buffer, size_t buffer_length, gs_system_t *system, const gs_request_t *request, response_t *response);

void router_api_1_0_collections(gs_system_t *system, const gs_request_t *request, response_t *response)
{
    if (gs_request_is_method(request, GS_GET) || gs_request_is_method(request, GS_POST))
    {
        gs_gridstore_t *gridstore = gs_system_get_gridstore(system);
        gs_collections_t *collections = gs_gridstore_get_collections(gridstore);
        char buffer[10240];

        if (gs_request_is_method(request, GS_GET)) {
            process_list_collections(buffer, ARRAY_LEN_OF(buffer), collections, response);
        } else {
            process_create_collections(buffer, ARRAY_LEN_OF(buffer), system, request, response);
        }
    } else {
        response_end(response, HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
    }
}

static void process_list_collections(char *buffer, size_t buffer_length, gs_collections_t *collections, response_t *response)
{
    FILE *memfile = fmemopen(buffer, buffer_length, "w");
    gs_collections_print(memfile, collections);
    fclose(memfile);

    response_content_type_set(response, MIME_CONTENT_TYPE_APPLICATION_JSON);
    response_field_set(response, "Connection", "close");
    response_body_set(response, buffer);
    response_end(response, HTTP_STATUS_CODE_200_OK);
}

static void process_create_collections(char *buffer, size_t buffer_length, gs_system_t *system, const gs_request_t *request, response_t *response)
{
    if (!gs_request_has_form(request, API_COLLECTIONS_CREATE_COLLECTION_KEY)) {
        response_end(response, HTTP_STATUS_CODE_400_BAD_REQUEST);
    } else {
        const char *collection_data;
        gs_request_form_by_name(&collection_data, request, API_COLLECTIONS_CREATE_COLLECTION_KEY);
        assert (collection_data);

        json_value *value = json_parse(collection_data, strlen(collection_data));

        json_value_free(value);


        sprintf(buffer, "OKAY %s", "YEAH");

        response_content_type_set(response, MIME_CONTENT_TYPE_APPLICATION_JSON);
        response_field_set(response, "Connection", "close");
        response_body_set(response, buffer);
        response_end(response, HTTP_STATUS_CODE_200_OK);
    }

}