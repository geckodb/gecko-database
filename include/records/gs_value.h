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

typedef uint8_t     u8_t;
typedef uint16_t    u16_t;
typedef uint32_t    u32_t;
typedef uint64_t    u64_t;
typedef int8_t      s8_t;
typedef int16_t     s16_t;
typedef int32_t     s32_t;
typedef int64_t     s64_t;
typedef float       float32_t;
typedef double      float64_t;
typedef gs_string_t string_t;
typedef gs_object_t object_t;
typedef gs_array_t  array_t;
typedef bool        boolean_t;

__END_DECLS