#pragma once

#include <gecko-commons/stdinc.h>
#include <gecko-commons/gs_base_relative_ptr.h>

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
    gs_base_relative_ptr_t           parent;
    gs_value_parent_type_e      parent_type;
    gs_value_type_e             value_type;
} gs_value_header_t;

__BEGIN_DECLS

GS_DECLARE(size_t)          gs_value_sizeof(gs_value_type_e type, const void *data);

GS_DECLARE(gs_status_t)     gs_value_new_generic(gs_base_relative_ptr_t *result,
                                                 BYTE *dst, gs_value_type_e type, gs_base_relative_ptr_t parent,
                                                 gs_value_parent_type_e parent_type, const void *data);

GS_DECLARE(gs_status_t)     gs_value_get_type(gs_value_type_e *type, const BYTE *ptr);
GS_DECLARE(gs_status_t)     gs_value_get_parent(gs_base_relative_ptr_t parent, const BYTE *ptr);
GS_DECLARE(gs_status_t)     gs_value_get_parent_type(gs_value_parent_type_e *type, const BYTE *ptr);

GS_DECLARE(void *)          gs_value_read_generic(const BYTE *ptr);
GS_DECLARE(void *)          gs_value_write_generic(const BYTE *ptr, const void *data);

__END_DECLS