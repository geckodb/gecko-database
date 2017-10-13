#pragma once

#include <gs.h>

#define NUM_SERVERS 10

typedef struct gs_system_t gs_system_t;

gs_status_t gs_system_create(gs_system_t **system, unsigned short gateway_port);

gs_status_t gs_system_start(gs_system_t *system);

gs_status_t gs_system_cleanup(gs_system_t *system);