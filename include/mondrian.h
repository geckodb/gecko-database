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
#include <async.h>
#include "progpool.h"

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct mondrian_t mondrian_t;

typedef struct mvm_handle_t {
    mondrian_t *instance;
    future_t future;
} mvm_handle_t;

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

int mondrian_open(mondrian_t **instance);

int mondrian_show_debug_log(mondrian_t *instance, bool show);

int mondrian_install(prog_id_t *out, mondrian_t *db, const program_t *program);

int mondrian_exec(mvm_handle_t *out, mondrian_t *db, prog_id_t prog_id, future_eval_policy run_policy);

progpool_t *mondrian_get_progpool(mondrian_t *db);

int mondrian_exec_by_name(mvm_handle_t *out, mondrian_t *db, const char *prog_name, future_eval_policy run_policy);

int mondrian_waitfor(const mvm_handle_t *handle);

int mondrian_close(mondrian_t *instance);

const db_config_t *mondrian_config(mondrian_t *instance);
