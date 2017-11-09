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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs.h>
#include <gs_event.h>

#include <apr.h>
#include <apr_pools.h>
#include <apr_queue.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define MESSAGE_QUEUE_SIZE_MAX 100
#define DISPATCHER_NUM_ROUTERS  50

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

typedef struct gs_dispatcher_t gs_dispatcher_t;

GS_DECLARE(gs_status_t) gs_dispatcher_create(gs_dispatcher_t **dispatcher);

GS_DECLARE(gs_status_t) gs_dispatcher_dispose(gs_dispatcher_t **dispatcher);

GS_DECLARE(gs_status_t) gs_dispatcher_start(gs_dispatcher_t *dispatcher);

GS_DECLARE(gs_status_t) gs_dispatcher_shutdown(gs_dispatcher_t *dispatcher, gs_system_t *system);

GS_DECLARE(gs_status_t) gs_dispatcher_connect(gs_dispatcher_t *dispatcher, gs_signal_type_e signal,
                                              gs_event_handler_t handler);

GS_DECLARE(gs_status_t) gs_dispatcher_publish(gs_dispatcher_t *dispatcher, gs_event_t *event);

GS_DECLARE(gs_status_t) gs_dispatcher_waitfor(gs_dispatcher_t *dispatcher, gs_event_t *event, gs_system_t *system);

__END_DECLS