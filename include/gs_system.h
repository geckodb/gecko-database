#pragma once

#include <gs.h>

#define NUM_SERVERS 10

typedef struct gs_gridstore_t gs_gridstore_t; /* forwarded */
typedef struct gs_dispatcher_t gs_dispatcher_t; /* forwarded */
typedef struct gs_shell_t gs_shell_t; /* forwarded */
typedef struct gs_server_pool_t gs_server_pool_t; /* forwarded */

typedef struct gs_system_t gs_system_t;

gs_status_t gs_system_create(gs_system_t **system, unsigned short gateway_port);

gs_status_t gs_system_start(gs_system_t *system);

gs_status_t gs_system_cleanup(gs_system_t *system);

gs_gridstore_t *gs_system_get_gridstore(const gs_system_t *system);

gs_dispatcher_t *gs_system_get_dispatcher(const gs_system_t *system);

gs_shell_t *gs_system_get_shell(const gs_system_t *system);

gs_server_pool_t *gs_system_get_server_pool(const gs_system_t *system);