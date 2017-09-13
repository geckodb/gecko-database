// Asynchronous program execution
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
#include <stdatomic.h>
#include "c11threads.h"

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    future_eager,
    future_lazy,
    future_sync
} future_eval_policy;

typedef enum {
    promise_pending,
    promise_fulfilled,
    promise_rejected
} promise_state;

typedef enum {
    resolved,
    rejected
} promise_result;

typedef void *(*promise_t)(promise_result *return_value, const void *capture);

struct _future_t {
    const void *capture;
    promise_t func;
    promise_state promise_state;
    atomic_bool promise_settled;
    thrd_t *thread;
    void *thread_args;
    future_eval_policy eval_policy;
    void *call_result;
};

typedef struct _future_t *future_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

future_t future_new(const void *capture, promise_t func, future_eval_policy policy);
void future_wait_for(future_t future);
const void *future_resolve(promise_result *return_type, future_t future);