#pragma once

#include <stdinc.h>
#include <inet/request.h>
#include <inet/response.h>

static inline void router_catch(capture_t *capture, const request_t *request, response_t *response)
{
    response_end(response, HTTP_STATUS_CODE_400_BAD_REQUEST);
}