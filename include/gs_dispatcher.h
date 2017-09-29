#pragma once

#include <gs.h>
#include <gs_event.h>

#include <apr.h>
#include <apr_pools.h>
#include <apr_queue.h>

#define MESSAGE_QUEUE_SIZE_MAX 100

__BEGIN_DECLS

typedef struct gs_dispatcher_t gs_dispatcher_t;

GS_DECLARE(gs_status_t) gs_dispatcher_create(gs_dispatcher_t **dispatcher);

GS_DECLARE(gs_status_t) gs_dispatcher_dispose(gs_dispatcher_t **dispatcher);

GS_DECLARE(gs_status_t) gs_dispatcher_start(gs_dispatcher_t *dispatcher);

GS_DECLARE(gs_status_t) gs_dispatcher_shutdown(gs_dispatcher_t *dispatcher);

GS_DECLARE(gs_status_t) gs_dispatcher_connect(gs_dispatcher_t *dispatcher, gs_signal_type_e signal,
                                              gs_event_handler_t handler);

GS_DECLARE(gs_status_t) gs_dispatcher_publish(gs_dispatcher_t *dispatcher, gs_event_t *event);

__END_DECLS