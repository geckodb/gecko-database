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

#include <defs.h>
#include <stdatomic.h>
#include "c11threads.h"

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum mdb_async_eval_policy { AEP_EAGER, AEP_LAZY, AEP_PARENT_THREAD };

enum mdb_async_promise_state { APS_PENDING, APS_FULFILLED, APS_REJECTED };

enum mdb_async_promise_return { APR_RESOLVED, APR_REJECTED, APR_UNDEFINED };

typedef void *(*promise)(enum mdb_async_promise_return *return_value, const void *capture);

typedef struct
{
    const void *capture;
    promise promise_func;
    enum mdb_async_promise_state promise_state;
    atomic_bool promise_settled;
    thrd_t *thread;
    void *thread_args;
    enum mdb_async_eval_policy eval_policy;
    void *call_result;
} mdb_async_future;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

mdb_async_future *mdb_async_future_alloc(const void *capture, promise func, enum mdb_async_eval_policy policy);

void mdb_async_future_wait_for(mdb_async_future *future);

const void *mdb_async_future_resolve(enum mdb_async_promise_return *return_type, mdb_async_future *future);

bool mdb_async_future_free(mdb_async_future *future);