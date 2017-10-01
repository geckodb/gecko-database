#pragma once

#include <gs.h>

typedef struct gs_spinlock_t gs_spinlock_t;

__BEGIN_DECLS

GS_DECLARE(gs_status_t) gs_spinlock_create(volatile gs_spinlock_t **spinlock);

GS_DECLARE(gs_status_t) gs_spinlock_dispose(volatile gs_spinlock_t **spinlock);

GS_DECLARE(gs_status_t) gs_spinlock_lock(volatile gs_spinlock_t *spinlock);

GS_DECLARE(gs_status_t) gs_spinlock_unlock(volatile gs_spinlock_t *spinlock);

__END_DECLS