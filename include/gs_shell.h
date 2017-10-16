#pragma once

#include <gs.h>
#include "gs_system.h"

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E   F O R D W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_dispatcher_t gs_dispatcher_t;
typedef struct gs_event_t gs_event_t;

__BEGIN_DECLS

typedef struct gs_shell_t gs_shell_t;

#define STDIN_BUFFER_SIZE 1024

GS_DECLARE(gs_status_t) gs_shell_create(gs_shell_t **shell, gs_system_t *system);

GS_DECLARE(gs_status_t) gs_shell_start(gs_shell_t *shell, gs_system_t *system);

GS_DECLARE(gs_status_t) gs_shell_dispose(gs_shell_t **shell);

GS_DECLARE(gs_status_t) gs_shell_handle_events(const gs_event_t *event);

GS_DECLARE(gs_status_t) gs_shell_shutdown(gs_shell_t *shell);

__END_DECLS