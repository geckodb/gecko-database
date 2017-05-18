#pragma once

#include <defs.h>

enum mdb_async_eval_policy { AEP_EAGER, AEP_LAZY, AEP_PARENT_THREAD };

enum mdb_async_promise_state { APS_PENDING, APS_FULFILLED, APS_REJECTED };

enum mdb_async_promise_return { APR_RESOLVED, APR_REJECTED, APR_UNDEFINED };

typedef void *(*promise)(enum mdb_async_promise_return *return_value, const void *capture);

typedef struct
{
    const void *capture;
    promise promise_func;
    enum mdb_async_promise_state promise_state;
    bool promise_settled;
    void *call_result;
} mdb_async_future;

mdb_async_future *mdb_async_future_alloc(const void *capture, promise func);

bool mdb_async_future_invoke(mdb_async_future *future, enum mdb_async_eval_policy policy);

void mdb_async_future_wait_for(mdb_async_future *future);

const void *mdb_async_future_resolve(enum mdb_async_promise_return *return_type, mdb_async_future *future);

bool mdb_async_future_free(mdb_async_future *future);