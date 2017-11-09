#pragma once

#include <gs.h>
#include <inet/gs_request.h>
#include <gs_dispatcher.h>
#include <inet/gs_response.h>

/*
 * Get collections:
 *          curl -s http://localhost:${PORT}/api/1.0/collections
 * Add collections:
            curl http://localhost:51174/api/1.0/collections -X POST   -F collection='
            {
                [
                    {
                        "name"="MyCollection"
                        "options"=
                        {
                            "storage_default"="host/nsm"
                        }
                    },
                    {
                        "name"="MyCollection2"
                        "options"=
                        {
                            "storage_default"="host/dsm"
                        }
                    }
                ]
          }'
 */

void router_api_1_0_collections(gs_system_t *system, const gs_request_t *request, gs_response_t *response);