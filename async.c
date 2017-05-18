#include <async.h>
#include <require.h>
#include <error.h>

mdb_async_future *mdb_async_future_alloc(const void *capture, promise func)
{
    mdb_async_future *future = NULL;
    if (mdb_require_non_null(capture) && mdb_require_non_null(func) &&
        (future = mdb_require_malloc(sizeof(mdb_async_future)))) {
            future->call_result = NULL;
            future->capture = capture;
            future->promise_func = func;
            future->promise_settled = false;
            future->promise_state = APS_PENDING;
    }
    return future;
}

bool mdb_async_future_invoke(mdb_async_future *future, enum mdb_async_eval_policy policy) {
    enum mdb_async_promise_return return_value = APR_UNDEFINED;
    void *call_result = NULL;

    if (mdb_require_non_null(future) && mdb_require_non_null(future->promise_func)) {
        switch (policy) {
            case AEP_EAGER:
                return true;
            case AEP_LAZY:
                return true;
            case AEP_PARENT_THREAD:
                call_result = future->promise_func(&return_value, future->capture);
                future->promise_settled = true;
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
            default:
                error_set_last(EC_INTERNALERROR);
                return false;
        }
    } else return false;
}

void mdb_async_future_wait_for(mdb_async_future *future)
{
    if (mdb_require_non_null(future)) {
        while (!future->promise_settled);
    }
}

const void *mdb_async_future_resolve(enum mdb_async_promise_return *return_type, mdb_async_future *future)
{
    void *result = NULL;

    if (mdb_require_non_null(future)) {
        if (!future->promise_settled)
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