#include <inet/gs_request.h>
#include <inet/gs_response.h>
#include <apr_strings.h>
#include <apr_tables.h>

typedef enum gs_request_body_e {
    GS_BODY_UNKNOWN,
    GS_MULTIPART
} gs_request_body_e;

typedef struct gs_request_t {
    apr_pool_t *pool;
    char *original;
    apr_table_t *fields;
    apr_table_t *form_data;
    gs_http_method_e method;
    char *resource;
    bool is_valid;
    bool is_multipart;
    char *boundary;
    gs_request_body_e body_type;
} gs_request_t;

 void parse_request(gs_request_t *request, int socket_desc);

GS_DECLARE(gs_status_t) gs_request_create(gs_request_t **request, int socket_desc)
{
    gs_request_t *result = GS_REQUIRE_MALLOC(sizeof(gs_request_t));
    apr_pool_create(&result->pool, NULL);
    result->fields = apr_table_make(result->pool, 10);
    result->form_data = apr_table_make(result->pool, 10);
    result->is_valid = false;
    result->is_multipart = false;
    result->body_type = GS_BODY_UNKNOWN;
    result->method = GS_UNKNOWN;

    parse_request(result, socket_desc);

    *request = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_request_dispose(gs_request_t **request_ptr)
{
    GS_REQUIRE_NONNULL(request_ptr)
    GS_REQUIRE_NONNULL(*request_ptr)
    gs_request_t *request = *request_ptr;
    apr_table_clear(request->form_data);
    apr_table_clear(request->fields);
    apr_pool_destroy(request->pool);
    free (request);
    *request_ptr = NULL;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_request_raw(char **original, const gs_request_t *request)
{
    GS_REQUIRE_NONNULL(request);
    GS_REQUIRE_NONNULL(original);
    *original = request->original;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_request_has_field(const gs_request_t *request, const char *key)
{
    GS_REQUIRE_NONNULL(request);
    GS_REQUIRE_NONNULL(key);
    return (apr_table_get(request->fields, key) != NULL);
}

GS_DECLARE(gs_status_t) gs_request_field_by_name(char const ** value, const gs_request_t *request, const char *key)
{
    GS_REQUIRE_NONNULL(request);
    GS_REQUIRE_NONNULL(key);
    GS_REQUIRE_NONNULL(value);
    *value = apr_table_get(request->fields, key);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_request_has_form(const gs_request_t *request, const char *key)
{
    GS_REQUIRE_NONNULL(request);
    GS_REQUIRE_NONNULL(key);
    return (apr_table_get(request->form_data, key) != NULL);
}

GS_DECLARE(gs_status_t) gs_request_form_by_name(char const **value, const gs_request_t *request, const char *key)
{
    GS_REQUIRE_NONNULL(request);
    GS_REQUIRE_NONNULL(key);
    GS_REQUIRE_NONNULL(value);
    *value = apr_table_get(request->form_data, key);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_request_method(gs_http_method_e *method, const gs_request_t *request)
{
    GS_REQUIRE_NONNULL(request);
    GS_REQUIRE_NONNULL(method);
    *method = request->method;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_request_is_method(const gs_request_t *request, gs_http_method_e method)
{
    GS_REQUIRE_NONNULL(request);
    return (request->method == method);
}

GS_DECLARE(gs_status_t) gs_request_resource(char **resource, const gs_request_t *request)
{
    GS_REQUIRE_NONNULL(request);
    GS_REQUIRE_NONNULL(resource);
    *resource = request->resource;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_request_is_valid(const gs_request_t *request)
{
    GS_REQUIRE_NONNULL(request);
    return (request->is_valid);
}

 void parse_request(gs_request_t *request, int socket_desc)
{
    char message_buffer[10240];

    // Read the request from the client socket
    recv(socket_desc, message_buffer, sizeof(message_buffer), 0);
    request->original = apr_pstrdup(request->pool, message_buffer);

    // Parse line by line
    char *last_line;
    char *line = apr_strtok(apr_pstrdup(request->pool, message_buffer), "\r\n", &last_line );
    bool process_http_method = true;

    while(line != NULL) {
        // Example request
        //      POST / HTTP/1.1
        //      Host: localhost:35497
        //      User-Agent: curl/7.43.0
        //      Accept: */*
        //      Content-Length: 190
        //      Expect: 100-continue
        //      Content-Type: multipart/form-data; boundary=------------------------5c30c9ccb5c18fb1
        //      (empty line)

        // until an empty line is reached, the header is being parsed
        if(strlen(line)) {
            if (process_http_method) {
                // length of the sub string before the first whitespace, i.e., the method "POST", "GET",...
                int method_ends_at = strcspn(line, " ");
                if (method_ends_at > 0) {
                    char *method_str = apr_pstrndup(request->pool, line, method_ends_at);
                    if (!strcmp(method_str, "OPTIONS"))
                        request->method = GS_OPTIONS;
                    else if (!strcmp(method_str, "GET"))
                        request->method = GS_GET;
                    else if (!strcmp(method_str, "HEAD"))
                        request->method = GS_HEAD;
                    else if (!strcmp(method_str, "POST"))
                        request->method = GS_POST;
                    else if (!strcmp(method_str, "PUT"))
                        request->method = GS_PUT;
                    else if (!strcmp(method_str, "DELETE"))
                        request->method = GS_DELETE;
                    else if (!strcmp(method_str, "TRACE"))
                        request->method = GS_TRACE;
                    else if (!strcmp(method_str, "CONNECT"))
                        request->method = GS_CONNECT;
                    else
                        request->method = GS_UNKNOWN;

                    // length of the sub string before the next white space after method first whitespace, i.e., "<RESOURCE>"
                    int resource_ends_at = strcspn(line + method_ends_at + 1, " ");
                    if (resource_ends_at > 0) {
                        request->resource = apr_pstrndup(request->pool, line + method_ends_at + 1, resource_ends_at);
                        request->is_valid = true;
                    }
                }
                process_http_method = false;
            } else {
                // length of the sub string before the assignment, i.e., fields "Host", "User-Agent",...
                int assignment_at = strcspn(line, ":");
                if (assignment_at > 0) {
                    char *field_name = apr_pstrndup(request->pool, line, assignment_at);
                    char *field_value = apr_pstrndup(request->pool, line + assignment_at + 2, strlen(line) + 2 - assignment_at);
                    apr_table_add(request->fields, field_name, field_value);
                }
            }
        }
        line = apr_strtok( NULL, "\r\n",  &last_line);
    }

    // in case the request requires an additional response to proceed, send this response
    const char *expect = apr_table_get(request->fields, "Expect");
    if (request->is_valid && expect && strlen(expect)) {
        int response_code_expected_at = strcspn(expect, "-");
        if (response_code_expected_at > 0 &&
            !strcmp(apr_pstrndup(request->pool, expect, response_code_expected_at), "100")) {

            // send 100 continue
            gs_response_t response;
            gs_response_create(&response);
            gs_response_end(&response, HTTP_STATUS_CODE_100_CONTINUE);
            char *response_text = gs_response_pack(&response);
            write(socket_desc, response_text, strlen(response_text));
            free (response_text);

            // read response, and set current line pointer to that content
            recv(socket_desc, message_buffer, sizeof(message_buffer), 0);
            line = apr_strtok(apr_pstrdup(request->pool, message_buffer), "\r\n", &last_line );
        }
    }

    // in case the request contains form-data resp. is multipart, read the subsequent data
    const char *content_type = apr_table_get(request->fields, "Content-Type");
    if (request->is_valid && content_type && strlen(content_type)) {
        // if the content is multipart, then there is a boundary definition which is starts after ";"
        if (strstr(content_type, ";")) {
            int multipart_at = strcspn(content_type, ";");
            if (multipart_at > 0 &&
                !strcmp(apr_pstrndup(request->pool, content_type, multipart_at), "multipart/form-data")) {
                int boundary_def_at = strcspn(content_type, "=");
                if (boundary_def_at > 0) {
                    request->is_multipart = true;
                    int boundary_def_prefix = boundary_def_at + 1;
                    request->boundary = apr_pstrndup(request->pool, content_type + boundary_def_prefix,
                                                     strlen(content_type) - boundary_def_prefix);
                    request->body_type = GS_MULTIPART;
                }
            }
        }
    }

    // parse body part
    switch (request->body_type) {
        case GS_MULTIPART: {
                bool read_name = true;
                while(line != NULL) {
                    if (!strstr(line, request->boundary)) {
                        char *attribute_name, *attribute_value;
                        if (read_name) {
                            if (strstr(line, "form-data; name=")) {
                                // read form name
                                int name_starts_at = strcspn(line, "\"") + 1;
                                int name_end_at = strcspn(line + name_starts_at, "\"");
                                if (name_end_at < strlen(line)) {
                                    attribute_name = apr_pstrndup(request->pool, line + name_starts_at, name_end_at);
                                    read_name = false;
                                }
                            }
                        } else {
                            // read form data
                            attribute_value = apr_pstrdup(request->pool, line);
                            read_name = true;
                        }
                        apr_table_add(request->form_data, attribute_name, attribute_value);
                    }
                    line = apr_strtok( NULL, "\r\n",  &last_line);
                }
            } break;
        default:
            request->is_valid = false;
            warn("Unknown body type for request: '%s'", request->original);
            break;
    }
}