#pragma once

#include <stdinc.h>
#include <containers/dict.h>

#define NUM_INIT_CONFIG_STATEMENTS    10

typedef struct {
    time_t last_read;
    dict_t *dict;
} pref_t;

void pref_load(
        pref_t *pref,
        const char *file,
        dict_t *dict,
        char * (*resolve_variables)(dict_t *dict, const char *string)
);

void pref_free(
        pref_t *pref
);

bool pref_has(
        const pref_t *pref,
        const char *key
);

const char *pref_get_str(
        const pref_t *pref,
        const char *key,
        const char *default_val
);

void pref_get_size_t(
        size_t *out,
        const pref_t *pref,
        const char *key,
        size_t *default_val
);
