#include <routers/api/1.0/gs_root.h>
#include <apr_strings.h>

#include <gs_system.h>
#include <gecko-http/gs_server.h>

static char *format_port_list(apr_pool_t *pool, gs_server_pool_t *server_pool);

void router_api_1_0_root(gs_system_t *system, const gs_request_t *request, gs_response_t *response)
{
    if (!gs_request_is_method(request, GS_GET)) {
        gs_response_end(response, HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
    } else {

       // gs_dispatcher_waitfor(dispatcher, gs_event_gridstore_invoke());

        gs_server_pool_t *server_pool = gs_system_get_server_pool(system);

        gs_response_content_type_set(response, "text/html");//MIME_CONTENT_TYPE_TEXT_PLAIN);
        gs_response_field_set(response, "Connection", "close");

        apr_pool_t *pool;
        apr_pool_create(&pool, NULL);

        const char *message = apr_pstrcat(pool,
                                          "{ \"result\": \"OK\", ",
                                            "\"ports\": [", format_port_list(pool, server_pool), "], "
                                            "\"{use-port}\": ", apr_ltoa(pool, gs_server_pool_next_port(server_pool)),
                                          " }\r\n"
                                          "\0", NULL);


        gs_response_body_set(response, message);
        gs_response_end(response, HTTP_STATUS_CODE_200_OK);

        apr_pool_destroy(pool);
    }
}

static char *format_port_list(apr_pool_t *pool, gs_server_pool_t *server_pool)
{
    size_t nports = gs_server_pool_get_num_of_servers(server_pool);
    unsigned short *ports = GS_REQUIRE_MALLOC(nports * sizeof(unsigned short));
    unsigned short *ports_it = ports;
    gs_server_pool_cpy_port_list(ports, server_pool);

    char *port_list = apr_pstrdup(pool, "");
    while (nports--) {
        port_list = apr_pstrcat(pool, port_list, apr_ltoa(pool, *ports_it), (nports > 0 ? ", " : ""), NULL);
        ports_it++;
    }

    free (ports);
    return port_list;
}