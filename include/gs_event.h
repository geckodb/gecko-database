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
#include <gs_gridstore.h>
#include <gs_shell.h>
#include "gs_spinlock.h"

// ---------------------------------------------------------------------------------------------------------------------
// M A R C O S
// ---------------------------------------------------------------------------------------------------------------------

#define GS_EVENT_PROCEED() \
    return GS_PROCEED

#define GS_EVENT_CATCHED() \
    return GS_CATCHED

#define GS_EVENT_FILTER_BY_RECEIVER_TAG(required_tag)                                                                  \
{                                                                                                                      \
    gs_object_type_tag_e receiver_tag;                                                                                 \
    if (!gs_event_get_subject(&receiver_tag, NULL, event, GS_RECEIVER) || (receiver_tag != required_tag)) {            \
        GS_DEBUG("event %p (type %d) was skipped by receiver", event, required_tag);                                   \
        return GS_SKIPPED;                                                                                             \
    }                                                                                                                  \
}

#define GS_EVENT_GET_RECEIVER(type)                                                                                    \
({                                                                                                                     \
    type *self;                                                                                                        \
    gs_event_get_subject(NULL, (void *) &self, event, GS_RECEIVER);                                                    \
    self;                                                                                                              \
})

#define GS_EVENT_GET_SIGNAL()                                                                                          \
({                                                                                                                     \
    gs_event_get_signal(event);                                                                                        \
})

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_dispatcher_t gs_dispatcher_t; /* forwarding */
typedef struct gs_server_t gs_server_t; /* forwarding */
typedef struct gs_event_t gs_event_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum gs_signal_type_e {
    GS_SIG_HEARTBEAT,
    GS_SIG_SYSEXIT,
    GS_SIG_SHUTDOWN,
    GS_SIG_TEST,
    GS_SIG_INVOKE
} gs_signal_type_e;

typedef enum gs_object_type_tag_e {
    GS_OBJECT_TYPE_NONE,
    GS_OBJECT_TYPE_SYSTEM,
    GS_OBJECT_TYPE_GRIDSTORE,
    GS_OBJECT_TYPE_DISPATCHER,
    GS_OBJECT_TYPE_SHELL,
    GS_OBJECT_TYPE_SERVER,
    GS_OBJECT_TYPE_EVENT_WRAPPER,
} gs_object_type_tag_e;

typedef enum gs_subject_kind_e {
    GS_SENDER,
    GS_RECEIVER
} gs_subject_kind_e;


typedef gs_status_t (*gs_event_handler_t)(const gs_event_t *event);

typedef void (*gs_event_dispose)(gs_event_t *self);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

gs_event_t *gs_event_new(gs_signal_type_e s, gs_object_type_tag_e t, void *sdr, gs_object_type_tag_e rcvr_t,
                         void *rcvr, void *data, gs_event_dispose d);

gs_event_t *gs_event_new_blocking(volatile gs_spinlock_t *lock, gs_event_t *contained_event);

gs_event_t *gs_event_heartbeat_new(gs_dispatcher_t *dispatcher);

void gs_event_free(gs_event_t *event);

gs_signal_type_e gs_event_get_signal(const gs_event_t *event);

void *gs_event_get_data(const gs_event_t *event);

gs_status_t gs_event_get_subject(gs_object_type_tag_e *type_tag, void **ptr, const gs_event_t *event, gs_subject_kind_e subj);

gs_event_t *gs_event_system_exit(gs_dispatcher_t *dispatcher, gs_object_type_tag_e sender_tag, void *sender);

gs_event_t *gs_event_dispatcher_shutdown(gs_dispatcher_t *dispatcher);

gs_event_t *gs_event_shell_shutdown(gs_dispatcher_t *dispatcher, gs_shell_t *shell);

gs_event_t *gs_event_server_shutdown(gs_dispatcher_t *dispatcher, gs_server_t *server);

gs_event_t *gs_event_gridstore_shutdown(gs_dispatcher_t *dispatcher, gs_gridstore_t *gridstore);

gs_event_t *gs_event_gridstore_test(gs_gridstore_t *gridstore);

gs_event_t *gs_event_gridstore_invoke();

