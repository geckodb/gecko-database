#pragma once

#include <stdinc.h>
#include <mvm.h>
#include <async.h>

typedef struct mondrian_t mondrian_t;

typedef struct mvm_handle_t {
    mondrian_t *instance;
    future_t future;
} mvm_handle_t;

int mondrian_open(mondrian_t **instance);

int mondrian_exec(mvm_handle_t *out, mondrian_t *instance, const program_t *program, future_eval_policy run_policy);

int mondrian_waitfor(const mvm_handle_t *handle);

int mondrian_close(mondrian_t *instance);

const db_config_t *mondrian_config(mondrian_t *instance);
