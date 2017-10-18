// Asynchronous program execution
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
#include <stdatomic.h>
#include "c11threads.h"

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_future_t *gs_future_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    future_eager,
    future_lazy,
    future_sync
} gs_future_eval_policy_e;

typedef enum {
    promise_pending,
    promise_fulfilled,
    promise_rejected
} gs_promise_state_e;

typedef enum {
    resolved,
    rejected
} gs_promise_result_e;

typedef void *(*promise_t)(gs_promise_result_e *return_value, const void *capture);

struct gs_future_t {
    const void *capture;
    promise_t func;
    gs_promise_state_e promise_state;
    atomic_bool promise_settled;
    thrd_t *thread;
    void *thread_args;
    gs_future_eval_policy_e eval_policy;
    void *call_result;
};

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

gs_future_t gs_future_new(const void *capture, promise_t func, gs_future_eval_policy_e policy);
void gs_future_wait_for(gs_future_t future);
const void *gs_future_resolve(gs_promise_result_e *return_type, gs_future_t future);