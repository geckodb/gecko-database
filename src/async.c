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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <async.h>
#include <require.h>
#include <error.h>
#include <c11threads.h>
#include <assert.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static bool _eval(future_t future, future_eval_policy policy);
static bool _sync_exec(void *, future_t, promise_result);
static bool _lazy_exec(void *, future_t, promise_result);
static bool _eager_exec(void *, future_t, promise_result);
static int  _sync_exec_wrapper(void *);
static void _start_thread(future_t future);
static void _future_free(future_t future);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

future_t future_new(const void *capture, promise_t func, future_eval_policy policy)
{
    GS_REQUIRE_NONNULL(capture)
    GS_REQUIRE_NONNULL(func);
    future_t future = NULL;

    if ((future = GS_REQUIRE_MALLOC(sizeof(struct _future_t)))) {
            future->call_result = NULL;
            future->capture = capture;
            future->func = func;
            future->promise_state = promise_pending;
            future->eval_policy = future_sync;
            future->thread = future->thread_args = NULL;
            atomic_store(&future->promise_settled, false);
    }
    return (future != NULL ? (_eval(future, policy) ? future : NULL) : NULL);
}

void future_wait_for(future_t future)
{
    GS_REQUIRE_NONNULL(future)
    if (future->eval_policy == future_lazy) {
        _start_thread(future);
    }
    while (!future->promise_settled);
}

const void *future_resolve(promise_result *return_type, future_t future)
{
    void *result = NULL;
    GS_REQUIRE_NONNULL(future)
    GS_REQUIRE_NONNULL(future->thread_args)

    if (!atomic_load(&future->promise_settled))
        future_wait_for(future);

    if (return_type != NULL) {
        switch (future->promise_state) {
            case promise_fulfilled:
                *return_type = resolved;
                break;
            case promise_rejected:
                *return_type = rejected;
                break;
            default:
                error(err_internal);
                return NULL;
        }
    }
    result = future->call_result;
    _future_free(future);

    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool _eval(future_t future, future_eval_policy policy)
{
    GS_REQUIRE_NONNULL(future)
    GS_REQUIRE_NONNULL(future->func)

    promise_result return_value = rejected;
    void *call_result = NULL;

    future->eval_policy = policy;

    switch (policy) {
        case future_eager:
            _eager_exec(call_result, future, return_value);
            return true;
        case future_lazy:
            _lazy_exec(call_result, future, return_value);
            return true;
        case future_sync:
            return _sync_exec(call_result, future, return_value);
        default:
            error(err_internal);
            return false;
    }
}

bool _sync_exec(void *call_result, future_t future, promise_result return_value)
{
    call_result = future->func(&return_value, future->capture);
    atomic_store(&future->promise_settled, true);
    switch (return_value) {
        case rejected:
            future->promise_state = promise_rejected;
            break;
        case resolved:
            future->promise_state = promise_fulfilled;
            break;
        default:
            error(err_internal);
            return false;
    }
    future->call_result = call_result;
    return true;
}

typedef struct
{
    void *call_result;
    future_t future;
    promise_result return_value;
} _this_exec_args;

static int _sync_exec_wrapper(void * args)
{
    assert (args != NULL);
    _this_exec_args *exec_args = (_this_exec_args *) args;
    bool result = _sync_exec(exec_args->call_result, exec_args->future, exec_args->return_value);
    assert (result);
    return 0;
}

bool _lazy_exec(void *call_result, future_t future, promise_result return_value)
{
    thrd_t *thread;
    _this_exec_args* args;
    GS_REQUIRE_NONNULL(future)

    if ((args = GS_REQUIRE_MALLOC(sizeof(_this_exec_args))) && (thread = GS_REQUIRE_MALLOC(sizeof(thrd_t)))) {
        args->call_result = call_result;
        args->future = future;
        args->return_value = return_value;
        future->thread = thread;
        future->thread_args = args;
        return true;
    } else return false;
}

bool _eager_exec(void *call_result, future_t future, promise_result return_value)
{
    if (_lazy_exec(call_result, future, return_value)) {
        _start_thread(future);
        return true;
    } else return false;
}

void _start_thread(future_t future)
{
    assert (future->thread != NULL && future->thread_args != NULL);
    thrd_create(future->thread, &_sync_exec_wrapper, future->thread_args);
    thrd_detach(*future->thread);
}

void _future_free(future_t future)
{
    GS_REQUIRE_NONNULL(future)
    if (future->call_result != NULL)
        free(future->call_result);
    if (future->thread_args)
        free (future->thread_args);
    free (future);
}