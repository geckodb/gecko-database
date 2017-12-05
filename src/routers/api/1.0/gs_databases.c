#include <json-parser/json.h>

#include <routers/api/1.0/gs_databases.h>
#include <gs_platform.h>
#include <gs_collections.h>
#include <gs_system.h>
#include <gs_gridstore.h>

/* curl http://localhost:53975/api/1.0/databases -X PUT */

#define API_DATABASES_CREATE_DATABASE_KEY "create-database"

static void process_modification_request(gs_system_t *system, const gs_request_t *request, gs_response_t *response);
static void process_read_request(gs_system_t *system, const gs_request_t *request, gs_response_t *response);

void router_api_1_0_databases(gs_system_t *system, const gs_request_t *request, gs_response_t *response)
{
    bool is_post = gs_request_is_method(request, GS_POST);
    bool is_get = gs_request_is_method(request, GS_GET);

    char *xxx;
    gs_request_raw(&xxx, request);
    GS_DEBUG("%s", xxx)

    if (is_get || is_post)
    {
        if (is_post) {
            process_modification_request(system, request, response);
        } else if (is_get) {
            process_read_request(system, request, response);
        }
//        gs_gridstore_t *gridstore = gs_system_get_gridstore(system);
        /*gs_collections_t *collections = gs_gridstore_get_collections(gridstore);
        char buffer[10240];
        memset(buffer, 0, 10240);

        if (gs_request_is_method(request, GS_GET)) {
            process_list_collections(buffer, ARRAY_LEN_OF(buffer), collections, response);
        } else {
            process_create_collections(buffer, ARRAY_LEN_OF(buffer), system, request, response);
        }*/
    } else {
        gs_response_end(response, HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
    }
}

static void process_modification_request(gs_system_t *system, const gs_request_t *request, gs_response_t *response)
{
    GS_EMPTY_CHAR_BUFFER(buffer, 10240)

    if (!gs_request_has_form(request, API_DATABASES_CREATE_DATABASE_KEY)) {
        gs_response_end(response, HTTP_STATUS_CODE_400_BAD_REQUEST);
    } else {
        const char *create_database_args;
        gs_request_form_by_name(&create_database_args, request, API_DATABASES_CREATE_DATABASE_KEY);
        fprintf(stderr, "%s\n", create_database_args);

        sprintf(buffer, "OKAY %s", "YEAH");

        gs_response_content_type_set(response, MIME_CONTENT_TYPE_APPLICATION_JSON);
        gs_response_field_set(response, "Connection", "close");
        gs_response_body_set(response, buffer);
        gs_response_end(response, HTTP_STATUS_CODE_200_OK);
    }

}

static void process_read_request(gs_system_t *system, const gs_request_t *request, gs_response_t *response)
{

}

/*static void process_list_collections(char *buffer, size_t buffer_length, gs_collections_t *collections, gs_response_t *response)
{
    FILE *memfile = fmemopen(buffer, buffer_length, "w");
    gs_collections_print(memfile, collections);
    fclose(memfile);

    gs_response_content_type_set(response, MIME_CONTENT_TYPE_APPLICATION_JSON);
    gs_response_field_set(response, "Connection", "close");
    gs_response_body_set(response, buffer);
    gs_response_end(response, HTTP_STATUS_CODE_200_OK);
}

static void process_create_collections(char *buffer, size_t buffer_length, gs_system_t *system, const gs_request_t *request, gs_response_t *response)
{
    if (!gs_request_has_form(request, API_DATABASES_CREATE_DATABASE_KEY)) {
        gs_response_end(response, HTTP_STATUS_CODE_400_BAD_REQUEST);
    } else {
        const char *collection_data;
        gs_request_form_by_name(&collection_data, request, API_DATABASES_CREATE_DATABASE_KEY);
        assert (collection_data);

        json_value *value = json_parse(collection_data, strlen(collection_data));

        fprintf(stderr, "%s\n", collection_data);

        json_value_free(value);


        sprintf(buffer, "OKAY %s", "YEAH");

        gs_response_content_type_set(response, MIME_CONTENT_TYPE_APPLICATION_JSON);
        gs_response_field_set(response, "Connection", "close");
        gs_response_body_set(response, buffer);
        gs_response_end(response, HTTP_STATUS_CODE_200_OK);
    }

}*/