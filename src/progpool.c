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

#include <progpool.h>

typedef struct pool_entry_t {
    bool deleted;
    const program_t *program;
} pool_entry_t;

struct progpool_t {
    pthread_mutex_t mutex;
    vec_t *entries;
};

int progpool_create(progpool_t **pool)
{
    *pool = GS_REQUIRE_MALLOC(sizeof(progpool_t));
    if (*pool == NULL)
        return MONDRIAN_ERROR;
    else {
        (*pool)->entries = vec_new(sizeof(pool_entry_t), 10);
        return ((*pool)->entries != NULL ? MONDRIAN_OK : MONDRIAN_ERROR);
    }
}

int progpool_dispose(progpool_t *pool)
{
    vec_free(pool->entries);
    free(pool);
    return MONDRIAN_OK;
}

int progpool_install(prog_id_t *out, progpool_t *pool, const program_t *program)
{
    if (progpool_find_by_name(NULL, pool, program_name(program)) == MONDRIAN_OK) {
        return MONDRIAN_ERROR;
    } else {
        if (out != NULL) {
            *out = vec_length(pool->entries);
        }
        pool_entry_t entry = {
            .deleted = false,
            .program = program_cpy(program)
        };
        vec_pushback(pool->entries, 1, &entry);
        return MONDRIAN_OK;
    }
}

int progpool_list(prog_id_t **ids, size_t *num_progs, const progpool_t *pool)
{
    if (ids == NULL || num_progs == NULL || pool == NULL) {
        return MONDRIAN_ERROR;
    } else {
        *num_progs = pool->entries->num_elements;
        *ids = GS_REQUIRE_MALLOC(*num_progs * sizeof(prog_id_t));
        const pool_entry_t *entries = (const pool_entry_t *) pool->entries->data;
        for (size_t i = 0, j = 0; i < *num_progs; i++) {
            if (!entries[i].deleted) {
                (*ids)[j] = i;
            }
        }
        return MONDRIAN_OK;
    }
}

int progpool_uninstall(progpool_t *pool, prog_id_t id)
{
    if (pool == NULL || id >= pool->entries->num_elements) {
        return MONDRIAN_ERROR;
    } else {
        pool_entry_t *entries = (pool_entry_t *) pool->entries->data;
        if (!entries[id].deleted) {
            entries[id].deleted = true;
            return MONDRIAN_OK;
        } else {
            return MONDRIAN_ERROR;
        }
    }
}

int progpool_find_by_name(prog_id_t *out, progpool_t *pool, const char *name)
{
    if (pool == NULL || name == NULL) {
        return MONDRIAN_ERROR;
    } else {
        const pool_entry_t *entries = (const pool_entry_t *) pool->entries->data;
        size_t num_entries = pool->entries->num_elements;
        for (size_t i = 0; i < num_entries; i++) {
            if (!entries[i].deleted && !strcmp(name, program_name(entries[i].program))) {
                if (out != NULL) {
                    *out = i;
                }
                return MONDRIAN_OK;
            }
        }
        return MONDRIAN_NOSUCHELEM;
    }
}

const program_t *progpool_get(progpool_t *pool, prog_id_t id)
{
    if (pool == NULL || id >= pool->entries->num_elements) {
        return NULL;
    } else {
        const pool_entry_t *entries = (const pool_entry_t *) pool->entries->data;
        return entries[id].deleted ? NULL : entries[id].program;
    }
}

int progpool_lock_exclusive(progpool_t *pool)
{
    if (pool != NULL) {
        pthread_mutex_lock(&pool->mutex);
        return MONDRIAN_OK;
    } return MONDRIAN_ERROR;
}

int progpool_unlock_exclusive(progpool_t *pool)
{
    if (pool != NULL) {
        pthread_mutex_unlock(&pool->mutex);
        return MONDRIAN_OK;
    } return MONDRIAN_ERROR;
}