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

static bool mdb_async_future_invoke(mdb_async_future *future, enum mdb_async_eval_policy policy);
static bool _this_exec(void *, mdb_async_future *, enum mdb_async_promise_return);
static bool _lazy_exec(void *, mdb_async_future *, enum mdb_async_promise_return);
static bool _eager_exec(void *, mdb_async_future *, enum mdb_async_promise_return);
static int _thrd_start_this_exec_wrapper(void *);
static void _start_thread(mdb_async_future *future);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

mdb_async_future *mdb_async_future_alloc(const void *capture, promise func, enum mdb_async_eval_policy policy)
{
    mdb_async_future *future = NULL;
    if (mdb_require_non_null(capture) && mdb_require_non_null(func) &&
        (future = mdb_require_malloc(sizeof(mdb_async_future)))) {
            future->call_result = NULL;
            future->capture = capture;
            future->promise_func = func;
            future->promise_state = APS_PENDING;
            future->eval_policy = AEP_PARENT_THREAD;
            future->thread = future->thread_args = NULL;
            atomic_store(&future->promise_settled, false);
    }
    return (future != NULL ? (mdb_async_future_invoke(future, policy) ? future : NULL) : NULL);
}

void mdb_async_future_wait_for(mdb_async_future *future)
{
    if (mdb_require_non_null(future)) {
        if (future->eval_policy == AEP_LAZY) {
            _start_thread(future);
        }
        while (!future->promise_settled);
    }
}

const void *mdb_async_future_resolve(enum mdb_async_promise_return *return_type, mdb_async_future *future)
{
    void *result = NULL;

    if (mdb_require_non_null(future)) {
        if (!atomic_load(&future->promise_settled))
            mdb_async_future_wait_for(future);

        if (return_type != NULL) {
            switch (future->promise_state) {
                case APS_FULFILLED:
                    *return_type = APR_RESOLVED;
                    break;
                case APS_REJECTED:
                    *return_type = APR_REJECTED;
                    break;
                default:
                    error_set_last(EC_INTERNALERROR);
                    return NULL;
            }
        }
        result = future->call_result;
    }
    return result;
}

bool mdb_async_future_free(mdb_async_future *future)
{
    bool is_non_null = mdb_require_non_null(future);
    if (is_non_null) {
        if (future->call_result != NULL)
            free(future->call_result);
        free (future);
    }
    return is_non_null;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool mdb_async_future_invoke(mdb_async_future *future, enum mdb_async_eval_policy policy)
{
    enum mdb_async_promise_return return_value = APR_UNDEFINED;
    void *call_result = NULL;

    if (mdb_require_non_null(future) && mdb_require_non_null(future->promise_func)) {
        future->eval_policy = policy;

        switch (policy) {
            case AEP_EAGER:
                _eager_exec(call_result, future, return_value);
                return true;
            case AEP_LAZY:
                _lazy_exec(call_result, future, return_value);
                return true;
            case AEP_PARENT_THREAD:
                return _this_exec(call_result, future, return_value);
            default:
                error_set_last(EC_INTERNALERROR);
                return false;
        }
    } else return false;
}

bool _this_exec(void *call_result, mdb_async_future *future, enum mdb_async_promise_return return_value)
{
    call_result = future->promise_func(&return_value, future->capture);
    atomic_store(&future->promise_settled, true);
    switch (return_value) {
        case APR_UNDEFINED:
            error_set_last(EC_INTERNALERROR);
            return false;
        case APR_REJECTED:
            future->promise_state = APS_REJECTED;
            break;
        case APR_RESOLVED:
            future->promise_state = APS_FULFILLED;
            break;
        default:
            error_set_last(EC_INTERNALERROR);
            return false;
    }
    future->call_result = call_result;
    return true;
}

typedef struct
{
    void *call_result;
    mdb_async_future *future;
    enum mdb_async_promise_return return_value;
} _this_exec_args;

static int _thrd_start_this_exec_wrapper(void * args)
{
    assert (args != NULL);
    _this_exec_args *exec_args = (_this_exec_args *) args;
    bool result = _this_exec(exec_args->call_result, exec_args->future, exec_args->return_value);
    assert (result);
    return 0;
}

bool _lazy_exec(void *call_result, mdb_async_future *future, enum mdb_async_promise_return return_value)
{
    thrd_t *thread;
    _this_exec_args* args;

    if (mdb_require_non_null(future) &&
       (args = mdb_require_malloc(sizeof(_this_exec_args))) && (thread = mdb_require_malloc(sizeof(thrd_t)))) {
        args->call_result = call_result;
        args->future = future;
        args->return_value = return_value;
        future->thread = thread;
        future->thread_args = args;
        return true;
    } else return false;
}

bool _eager_exec(void *call_result, mdb_async_future *future, enum mdb_async_promise_return return_value)
{
    if (_lazy_exec(call_result, future, return_value)) {
        _start_thread(future);
        return true;
    } else return false;
}

void _start_thread(mdb_async_future *future)
{
    assert (future->thread != NULL && future->thread_args != NULL);
    thrd_create(future->thread, &_thrd_start_this_exec_wrapper, future->thread_args);
    thrd_detach(*future->thread);
}
