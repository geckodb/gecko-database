#include <mondrian.h>
#include <containers/list.h>
#include <debug.h>

typedef struct vm_exec_info {
    future_t future;
    time_t start;
} vm_exec_info;

struct mondrian_t {
    db_config_t config;
    list_t *vm_programs;

    time_t start;
};

typedef struct mondrian_vm_exec_args_t {
    mondrian_t *instance;
    const program_t *program;
} mondrian_vm_exec_args_t;


void *promise_mondrian_vm_exec(promise_result *return_value, const void *capture);

//----------------------------------------------------------------------------------------------------------------------

int mondrian_open(mondrian_t **instance)
{
    assert (instance);
    *instance = malloc(sizeof(mondrian_t));
    (*instance)->start = time(NULL);
    (*instance)->vm_programs = list_create(sizeof(vm_exec_info));
    gs_db_config_load(&(*instance)->config);
    return MONDRIAN_OK;
}

int mondrian_exec(mvm_handle_t *out, mondrian_t *instance, const program_t *program, future_eval_policy run_policy)
{
    if (instance == NULL || program == NULL)
        return MONDRIAN_ERROR;
    else {
        future_t retval;

        mondrian_vm_exec_args_t *args = require_good_malloc(sizeof(mondrian_vm_exec_args_t));
        *args = (mondrian_vm_exec_args_t) {
            .instance = instance,
            .program = program
        };

        LOG_DEBUG(instance, "intent to execute mvm program %p received", program);

        retval = future_create(args, promise_mondrian_vm_exec, run_policy);
        vm_exec_info info = {
            .future = retval,
            .start  = time(NULL)
        };
        list_push(instance->vm_programs, &info);

        if (out != NULL) {
            out->future = retval;
            out->instance = instance;
        }

        return MONDRIAN_OK;
    }
}

int mondrian_waitfor(const mvm_handle_t *handle)
{
    if (handle == NULL || handle->instance == NULL || handle->future == NULL) {
        return MONDRIAN_ERROR;
    } else {
        future_wait_for(handle->future);
        return MONDRIAN_OK;
    }
}

int mondrian_close(mondrian_t *instance)
{
    assert (instance);
    //conf_free(instance->config);  // TODO: freeing up config causes freeing up unallocated pointer
    return MONDRIAN_OK;
}

const db_config_t *mondrian_config(mondrian_t *instance)
{
    return (&instance->config);
}

//----------------------------------------------------------------------------------------------------------------------

void *promise_mondrian_vm_exec(promise_result *return_value, const void *capture)
{
    assert(capture);
    mondrian_vm_exec_args_t *args = (mondrian_vm_exec_args_t *) capture;
    int result = 0;
    mondrian_vm_t *vm;
    panic_if((mondrian_vm_create(&vm, args->instance) != MONDRIAN_OK),
             "Internal error: unable to create virtual machine for program %p", args->program);

    LOG_DEBUG(args->instance, "mvm program %p is starting...", args->program);

    mondrian_vm_run(vm, args->program, &result);
    *return_value =  (result == 0 ? resolved : rejected);
    return NULL;
}