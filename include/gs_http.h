#pragma once

#include <gs.h>

typedef uint32_t http_status_code_t;

typedef enum content_type_t
{
    MEDIA_MULTI_PART_FORM_DATA
} content_type_t;

typedef enum method_t {
    HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE, HTTP_OTHERS
} method_t;

#define HTTP_STATUS_CODE_100_CONTINUE           100
#define HTTP_STATUS_CODE_200_OK                 200
#define HTTP_STATUS_CODE_500_INTERNAL_ERR       500
#define HTTP_STATUS_CODE_400_BAD_REQUEST        400
#define HTTP_STATUS_CODE_404_NOT_FOUND          404
#define HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED 405
#define HTTP_STATUS_CODE_408_REQUEST_TIMEOUT    408


#define HTTP_GET_STRING    "GET"
#define HTTP_PUT_STRING    "PUT"
#define HTTP_POST_STRING   "POST"
#define HTTP_DELETE_STRING "DELETE"

const char *codestr(http_status_code_t code);

const char *methodstr(http_status_code_t code);