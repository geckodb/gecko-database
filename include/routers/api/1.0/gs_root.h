#pragma once

#include <gecko-commons/gecko-commons.h>
#include <gecko-http/gs_response.h>
#include <gecko-http/gs_request.h>
#include <gs_dispatcher.h>

void router_api_1_0_root(gs_system_t *system, const gs_request_t *request, gs_response_t *response);