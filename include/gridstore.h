#pragma once

#include "error.h"

typedef struct {
    dict_t *config;
    time_t timestamp_start;
} gridstore_instance_t;

error_code gridstore_init(gridstore_instance_t *instance);

const char *gridstore_get_config(gridstore_instance_t *instance, const char *settings_name);

void gridstore_shutdown(gridstore_instance_t *instance);