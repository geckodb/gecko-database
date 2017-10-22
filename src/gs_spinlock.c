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
// If not, see .

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_spinlock.h>
#include <stdatomic.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_spinlock_t
{
    volatile atomic_int exclusion;
    volatile atomic_int skip;
} gs_spinlock_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

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

