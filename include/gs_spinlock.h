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

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E   F O R D W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_spinlock_t gs_spinlock_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

GS_DECLARE(gs_status_t) gs_spinlock_create(volatile gs_spinlock_t **spinlock);

GS_DECLARE(gs_status_t) gs_spinlock_dispose(volatile gs_spinlock_t **spinlock);

GS_DECLARE(gs_status_t) gs_spinlock_lock(volatile gs_spinlock_t *spinlock);

GS_DECLARE(gs_status_t) gs_spinlock_unlock(volatile gs_spinlock_t *spinlock);

__END_DECLS