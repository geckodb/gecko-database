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
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <inet/request.h>

#define HTTP_GET_STRING    "GET"
#define HTTP_PUT_STRING    "PUT"
#define HTTP_POST_STRING   "POST"
#define HTTP_DELETE_STRING "DELETE"

static inline bool strstart(const char *string, const char *substr)
{
    return (strncmp(string, substr, strlen(substr)) == 0);
}

void request_parse(request_t *request, const char *string) {
    REQUIRE_NONNULL(request)
    char *request_str = strdup(string);
    request_str[strstr(request_str, "\n") - request_str] = '\0';
    fprintf(stderr, "%s\n", request_str );

    char *method_str = NULL;
    if (strstart(request_str, HTTP_GET_STRING)) {
        request->method = HTTP_GET;
        method_str = HTTP_GET_STRING;
    } else if (strstart(request_str, HTTP_PUT_STRING)) {
        request->method = HTTP_PUT;
        method_str = HTTP_PUT_STRING;
    } else if (strstart(request_str, HTTP_POST_STRING)) {
        request->method = HTTP_POST;
        method_str = HTTP_POST_STRING;
    } else if (strstart(request_str, HTTP_DELETE_STRING)) {
        request->method = HTTP_DELETE;
        method_str = HTTP_DELETE_STRING;
    } else {
        request->method = HTTP_OTHERS;
        request->valid = false;
        return;
    }

    if (strlen(request_str) - strlen(method_str) > 0) {
        char *original = strdup(request_str + strlen(method_str) + 1);
        char *http_resource = strdup(original);

        if (strstr(request_str, "?") != NULL) {
            char *tail = strstr(http_resource, "?");
            http_resource[strlen(http_resource) - strlen(tail)] = '\0';
            request->resource = http_resource;
            request->params = strdup(original + strlen(http_resource) + 1);
            request->params[strlen(request->params) - strlen(strstr(request->params, "HTTP/1")) - 1] = '\0';
        } else {
            request->resource = http_resource;
            request->resource[strlen(request->resource) - strlen(strstr(request->resource, "HTTP/1")) - 1] = '\0';
            request->params = NULL;
        }

        request->valid = true;
    } else {
        request->valid = false;
    }
}

const char *methodstr(http_status_code_t code) {
    switch (code) {
        case HTTP_GET:    return HTTP_GET_STRING;
        case HTTP_POST:   return HTTP_POST_STRING;
        case HTTP_PUT:    return HTTP_PUT_STRING;
        case HTTP_DELETE: return HTTP_DELETE_STRING;
        default: return "(unknown)";
    }
}

void request_print(FILE *file, const request_t *request)
{
    REQUIRE_NONNULL(file)
    REQUIRE_NONNULL(request)
    printf("request{method=%s, is_valid='%d', resource='%s', params='%s'}\n",
           methodstr(request->method), request->valid, request->resource, request->params);
}