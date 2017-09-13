// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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

#include <stdinc.h>
#include <mvm.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct progpool_t progpool_t;

typedef u64 prog_id_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

int progpool_create(progpool_t **pool);

int progpool_free(progpool_t *pool);

int progpool_install(prog_id_t *out, progpool_t *pool, const program_t *program);

int progpool_list(prog_id_t **ids, size_t *num_progs, const progpool_t *pool);

int progpool_uninstall(progpool_t *pool, prog_id_t id);

int progpool_find_by_name(prog_id_t *out, progpool_t *pool, const char *name);

const program_t *progpool_get(progpool_t *pool, prog_id_t id);

int progpool_lock_exclusive(progpool_t *pool);

int progpool_unlock_exclusive(progpool_t *pool);