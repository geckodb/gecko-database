#pragma once

#include <stdinc.h>

__BEGIN_DECLS

typedef struct gs_event_t gs_event_t; /* forwarding */

typedef struct gs_gridstore_t gs_gridstore_t;

GS_DECLARE(gs_status_t) gs_gridstore_create(gs_gridstore_t **gridstore);

GS_DECLARE(gs_status_t) gs_gridstore_handle_events(const gs_event_t *event);

__END_DECLS