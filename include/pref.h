#pragma once

#include <time.h>
#include <containers/dict.h>

#define NUM_INIT_CONFIG_STATEMENTS    10

typedef struct {
    time_t last_read;
    dict_t *dict;
} pref_t;

void pref_create(
        pref_t *pref,
        const char *file
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

void pref_get_uint32(
        unsigned *out,
        const pref_t *pref,
        const char *key,
        unsigned *default_val
);
