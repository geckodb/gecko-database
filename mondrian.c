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

#include <mondrian.h>
#include <containers/list.h>
#include <debug.h>
#include <progpool.h>
#include <conf.h>

typedef struct vm_exec_info {
    future_t future;
    time_t start;
} vm_exec_info;

struct mondrian_t {
    db_config_t config;
    list_t *vm_programs;
    progpool_t *progpool;

    time_t start;
};

typedef struct mondrian_vm_exec_args_t {
    mondrian_t *instance;
    const program_t *program;
} mondrian_vm_exec_args_t;


static inline int mondrian_start_vm(mvm_handle_t *out, mondrian_t *instance, const program_t *program, future_eval_policy run_policy);

void *promise_mondrian_vm_exec(promise_result *return_value, const void *capture);

//----------------------------------------------------------------------------------------------------------------------

int mondrian_open(mondrian_t **instance)
{
    assert (instance);
    *instance = REQUIRE_MALLOC(sizeof(mondrian_t));
    (*instance)->start = time(NULL);
    (*instance)->vm_programs = list_create(sizeof(vm_exec_info));
    progpool_create(&((*instance)->progpool));
    gs_db_config_load(&(*instance)->config);
    return MONDRIAN_OK;
}

int mondrian_show_debug_log(mondrian_t *instance, bool show)
{
    if (instance == NULL) {
        return MONDRIAN_ERROR;
    } else {
        instance->config.log.debug = show;
        return MONDRIAN_OK;
    }
}

int mondrian_install(prog_id_t *out, mondrian_t *db, const program_t *program)
{
    if (db == NULL || program == NULL) {
        return MONDRIAN_ERROR;
    } else {
        return progpool_install(out, db->progpool, program);
    }
}

int mondrian_exec(mvm_handle_t *out, mondrian_t *db, prog_id_t prog_id, future_eval_policy run_policy)
{
    if (db == NULL) {
        return MONDRIAN_ERROR;
    } else {
        const program_t *program = progpool_get(db->progpool, prog_id);
        if (program != NULL) {
            return mondrian_start_vm(out, db, program, run_policy);
        } else return MONDRIAN_ERROR;
    }
}

progpool_t *mondrian_get_progpool(mondrian_t *db)
{
    return (db == NULL ? NULL : db->progpool);
}

int mondrian_exec_by_name(mvm_handle_t *out, mondrian_t *db, const char *prog_name, future_eval_policy run_policy)
{
    if (db == NULL || prog_name == NULL)
        return MONDRIAN_ERROR;
    else {
        prog_id_t id;
        if (progpool_find_by_name(&id, db->progpool, prog_name) == MONDRIAN_OK) {
            return mondrian_exec(out, db, id, run_policy);
        } else return MONDRIAN_ERROR;
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

static inline int mondrian_start_vm(mvm_handle_t *out, mondrian_t *instance, const program_t *program, future_eval_policy run_policy)
{
    if (instance == NULL || program == NULL)
        return MONDRIAN_ERROR;
    else {
        future_t retval;

        mondrian_vm_exec_args_t *args = REQUIRE_MALLOC(sizeof(mondrian_vm_exec_args_t));
        *args = (mondrian_vm_exec_args_t) {
                .instance = instance,
                .program = program
        };

        LOG_DEBUG(instance, "intent to execute mvm job '%s' (%p) received", program_name(program), program);

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

void *promise_mondrian_vm_exec(promise_result *return_value, const void *capture)
{
    assert(capture);
    mondrian_vm_exec_args_t *args = (mondrian_vm_exec_args_t *) capture;
    int result = 0;
    mondrian_vm_t *vm;
    panic_if((mondrian_vm_create(&vm, args->instance) != MONDRIAN_OK),
             "Internal error: unable to create virtual machine for program %p", args->program);

    LOG_DEBUG(args->instance, "mvm job %s (%p) is starting...", program_name(args->program), args->program);
    mondrian_vm_run(vm, args->program, &result);
    mondrian_vm_free(vm);
    *return_value =  (result == 0 ? resolved : rejected);
    return NULL;
}