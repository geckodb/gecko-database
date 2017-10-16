#pragma once

#include <gs.h>
#include <inet/response.h>
#include <inet/gs_request.h>
#include <gs_dispatcher.h>

/*
 * curl -s http://localhost:${SELECT_PORT}/api/1.0/collections
 */

void router_api_1_0_collections(gs_system_t *system, const gs_request_t *request, response_t *response);