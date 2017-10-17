#include <gs_http.h>

const char *gs_codestr(gs_http_status_code_t code)
{
    switch (code) {
        case HTTP_STATUS_CODE_100_CONTINUE:           return "100 Continue";
        case HTTP_STATUS_CODE_200_OK:                 return "200 OK";
        case HTTP_STATUS_CODE_400_BAD_REQUEST:        return "400 Bad Request";
        case HTTP_STATUS_CODE_408_REQUEST_TIMEOUT:    return "408 Request Timeout";
        case HTTP_STATUS_CODE_404_NOT_FOUND:          return "404 Not Found";
        case HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED: return "405 Method Not Allowed";
        case HTTP_STATUS_CODE_500_INTERNAL_ERR:
        default:                                      return "500 Internal Server Error";
    }
}

const char *gs_methodstr(gs_http_status_code_t code) {
    switch (code) {
        case HTTP_GET:    return HTTP_GET_STRING;
        case HTTP_POST:   return HTTP_POST_STRING;
        case HTTP_PUT:    return HTTP_PUT_STRING;
        case HTTP_DELETE: return HTTP_DELETE_STRING;
        default: return "(unknown)";
    }
}