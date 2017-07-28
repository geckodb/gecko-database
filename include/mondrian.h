#pragma once

#include <stdinc.h>
#include <mvm.h>
#include <async.h>
#include "progpool.h"

typedef struct mondrian_t mondrian_t;

typedef struct mvm_handle_t {
    mondrian_t *instance;
    future_t future;
} mvm_handle_t;

int mondrian_open(mondrian_t **instance);

int mondrian_show_debug_log(mondrian_t *instance, bool show);

int mondrian_install(prog_id_t *out, mondrian_t *db, const program_t *program);

int mondrian_exec(mvm_handle_t *out, mondrian_t *db, prog_id_t prog_id, future_eval_policy run_policy);

progpool_t *mondrian_get_progpool(mondrian_t *db);

int mondrian_exec_by_name(mvm_handle_t *out, mondrian_t *db, const char *prog_name, future_eval_policy run_policy);

int mondrian_waitfor(const mvm_handle_t *handle);

int mondrian_close(mondrian_t *instance);

const db_config_t *mondrian_config(mondrian_t *instance);
