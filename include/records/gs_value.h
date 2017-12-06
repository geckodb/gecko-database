#pragma once

#include <gecko-commons/stdinc.h>

typedef enum gs_value_type_e {
    gs_value_type_u8, gs_value_type_u16, gs_value_type_u32, gs_value_type_u64,
    gs_value_type_s8, gs_value_type_s16, gs_value_type_s32, gs_value_type_s64,
    gs_value_type_float32, gs_value_type_float64, gs_value_type_string,
    gs_value_type_object, gs_value_type_array, gs_value_type_boolean,
    gs_value_type_null
} gs_value_type_e;

typedef struct gs_value_t gs_value_t;

__BEGIN_DECLS

GS_DECLARE(gs_status_t) gs_value_new_ex(gs_value_t *value, gs_value_type_e type, const BYTE *data);

GS_DECLARE(gs_status_t) gs_value_new_u8(gs_value_t *value, );

__END_DECLS