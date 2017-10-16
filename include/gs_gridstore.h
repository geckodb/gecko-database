#pragma once

#include <gs.h>

__BEGIN_DECLS

typedef struct gs_event_t gs_event_t; /* forwarding */
typedef struct gs_collections_t gs_collections_t; /* forwarding */

typedef struct gs_gridstore_t gs_gridstore_t;

GS_DECLARE(gs_status_t) gs_gridstore_create(gs_gridstore_t **gridstore);

GS_DECLARE(gs_status_t) gs_gridstore_handle_events(const gs_event_t *event);

GS_DECLARE(gs_collections_t *) gs_gridstore_get_collections(const gs_gridstore_t *gridstore);

__END_DECLS