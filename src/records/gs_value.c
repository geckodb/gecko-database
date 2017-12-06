#include <gecko-commons/stdinc.h>
#include <records/gs_value.h>

#define DECLARE_PRIMITIVE_NEW(type)                                                                                    \
    static inline void *value_##type##_new(gs_value_type_e value_type, gs_base_relative_ptr_t parent,                  \
                                           gs_value_parent_type_e parent_type, const void *data,                       \
                                           gs_allocator_t allocator)                                                   \
    {                                                                                                                  \
        gs_value_##type *result = allocator(sizeof(gs_value_##type));                                                  \
        result->header = (gs_value_header_t) {                                                                         \
                .parent = parent,                                                                                      \
                .parent_type = parent_type,                                                                            \
                .value_type = value_type                                                                               \
        };                                                                                                             \
        result->value = *(type *) data;                                                                                \
        return result;                                                                                                 \
    };

DECLARE_PRIMITIVE_NEW(atomic_u8_t)

GS_DECLARE(void *) gs_value_new(gs_status_t *status, gs_value_type_e type,
                                gs_base_relative_ptr_t parent, gs_value_parent_type_e parent_type,
                                const void *data, gs_allocator_t allocator)
{
    void *result = NULL;
    if (status && data && allocator) {
        switch (type) {
            case gs_value_type_u8:  return value_atomic_u8_t_new(type, parent, parent_type, data, allocator);
            case gs_value_type_u16:
            case gs_value_type_u32:
            case gs_value_type_u64:
            case gs_value_type_s8:
            case gs_value_type_s16:
            case gs_value_type_s32:
            case gs_value_type_s64:
            case gs_value_type_float32:
            case gs_value_type_float64:
            case gs_value_type_string:
            case gs_value_type_object:
            case gs_value_type_array:
            case gs_value_type_boolean:
            case gs_value_type_null:
            default: {
                *status = GS_INTERNALERR;
                return NULL;
            }
        }
    }
    *status = result ? *status : GS_FAILED;
    return result;
}

