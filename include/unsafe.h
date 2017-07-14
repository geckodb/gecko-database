#pragma once

#include <stdinc.h>
#include <field_type.h>

size_t gs_unsafe_field_get_println(enum field_type type, const void *data);

char *gs_unsafe_field_to_string(enum field_type type, const void *data);