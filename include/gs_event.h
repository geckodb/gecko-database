#pragma once

#include <stdinc.h>
#include "gs_gridstore.h"

typedef struct gs_dispatcher_t gs_dispatcher_t; /* forwarding */

typedef enum gs_signal_type_e {
    GS_SIG_HEARTBEAT,
    GS_SIG_SHUTDOWN,
    GS_SIG_TEST
} gs_signal_type_e;

typedef enum gs_event_tag_e {
    GS_EVENT_TAG_GRIDSTORE,
    GS_EVENT_TAG_DISPATCHER
} gs_event_tag_e;

typedef struct gs_event_t gs_event_t;

typedef gs_status_t (*gs_event_handler_t)(const gs_event_t *event);

typedef void (*gs_event_dispose)(gs_event_t *self);

#define GS_EVENT_PROCEED() \
    return GS_PROCEED

#define GS_EVENT_CATCHED() \
    return GS_CATCHED

gs_event_t *gs_event_new(gs_signal_type_e s, gs_event_tag_e t, void *sdr, void *p, gs_event_dispose d);

void gs_event_free(gs_event_t *event);

gs_signal_type_e gs_event_get_signal(const gs_event_t *event);

void *gs_event_get_payload(const gs_event_t *event);

gs_event_t *gs_event_dispatcher_shutdown(gs_dispatcher_t *dispatcher);

gs_event_t *gs_event_heartbeat_new(gs_dispatcher_t *dispatcher);

gs_event_t *gs_event_gridstore_test(gs_gridstore_t *gridstore);


