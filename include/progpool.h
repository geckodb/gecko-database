#pragma once

#include <stdinc.h>
#include <mvm.h>

typedef struct progpool_t progpool_t;

typedef u64 prog_id_t;

int progpool_create(progpool_t **pool);

int progpool_free(progpool_t *pool);

int progpool_install(prog_id_t *out, progpool_t *pool, const program_t *program);

int progpool_list(prog_id_t **ids, size_t *num_progs, const progpool_t *pool);

int progpool_uninstall(progpool_t *pool, prog_id_t id);

int progpool_find_by_name(prog_id_t *out, progpool_t *pool, const char *name);

const program_t *progpool_get(progpool_t *pool, prog_id_t id);

int progpool_lock_exclusive(progpool_t *pool);

int progpool_unlock_exclusive(progpool_t *pool);