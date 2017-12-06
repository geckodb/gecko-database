#pragma once

#include <gecko-commons/stdinc.h>
#include <gecko-commons/gs_base_relative_ptr.h>

#define DECLARE_PRIMITIVE_VALUE(type_name)          \
    typedef struct gs_value_##type_name {           \
        gs_value_header_t           header;         \
        type_name                   value;          \
    } gs_value_##type_name;

#define DECLARE_COMPOSITE_VALUE(type_name)          \
    typedef struct gs_value_##type_name {           \
        gs_value_header_t           header;         \
        gs_base_relative_ptr_t      value_ptr;      \
    } gs_value_##type_name;

typedef enum gs_value_type_e {
    gs_value_type_u8, gs_value_type_u16, gs_value_type_u32, gs_value_type_u64,
    gs_value_type_s8, gs_value_type_s16, gs_value_type_s32, gs_value_type_s64,
    gs_value_type_float32, gs_value_type_float64, gs_value_type_string,
    gs_value_type_object, gs_value_type_array, gs_value_type_boolean,
    gs_value_type_null
} gs_value_type_e;

typedef enum gs_value_parent_type_e {
    gs_value_parent_type_array,
    gs_value_parent_type_property
} gs_value_parent_type_e;

typedef struct gs_value_header_t {
    gs_base_relative_ptr_t      parent;
    gs_value_parent_type_e      parent_type;
    gs_value_type_e             value_type;
} gs_value_header_t;

DECLARE_PRIMITIVE_VALUE(atomic_u8_t)
DECLARE_PRIMITIVE_VALUE(u16_t)
DECLARE_PRIMITIVE_VALUE(u32_t)
DECLARE_PRIMITIVE_VALUE(u64_t)
DECLARE_PRIMITIVE_VALUE(s8_t)
DECLARE_PRIMITIVE_VALUE(s16_t)
DECLARE_PRIMITIVE_VALUE(s32_t)
DECLARE_PRIMITIVE_VALUE(s64_t)
DECLARE_PRIMITIVE_VALUE(float32_t)
DECLARE_PRIMITIVE_VALUE(float64_t)
DECLARE_PRIMITIVE_VALUE(boolean_t)
DECLARE_COMPOSITE_VALUE(string_t)
DECLARE_COMPOSITE_VALUE(object_t)
DECLARE_COMPOSITE_VALUE(array_t)

typedef void *(*gs_allocator_t)(size_t size);

__BEGIN_DECLS

GS_DECLARE(void *)      gs_value_new(gs_status_t *status, gs_value_type_e type,
                                     gs_base_relative_ptr_t parent, gs_value_parent_type_e parent_type,
                                     const void *data, gs_allocator_t allocator);

__END_DECLS