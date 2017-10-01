#include <gs_spinlock.h>
#include <stdatomic.h>

typedef struct gs_spinlock_t
{
    volatile atomic_int exclusion;
    volatile atomic_int skip;
} gs_spinlock_t;

GS_DECLARE(gs_status_t) gs_spinlock_create(volatile gs_spinlock_t **spinlock)
{
    volatile gs_spinlock_t *result = GS_REQUIRE_MALLOC(sizeof(gs_spinlock_t));
    atomic_init(&result->skip, false);
    *spinlock = result;

    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_spinlock_dispose(volatile gs_spinlock_t **spinlock_ptr)
{
    GS_REQUIRE_NONNULL(spinlock_ptr)
    GS_REQUIRE_NONNULL(*spinlock_ptr)
    free ((void *) *spinlock_ptr);
    *spinlock_ptr = NULL;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_spinlock_lock(volatile gs_spinlock_t *spinlock)
{
    atomic_init(&spinlock->exclusion, true);
    while (atomic_load(&spinlock->exclusion) && !atomic_load(&spinlock->skip))
            ;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_spinlock_unlock(volatile gs_spinlock_t *spinlock)
{
    atomic_store(&spinlock->exclusion, false);
    atomic_store(&spinlock->skip, true);
    return GS_SUCCESS;
}

