#include <gecko-commons/stdinc.h>
#include <records/gs_value.h>



GS_DECLARE(size_t) gs_value_sizeof(gs_value_type_e type, const void *data)
{
    size_t required_size = sizeof(gs_value_header_t);
    switch (type) {
        case gs_value_type_u8:
            required_size += sizeof(atomic_u8_t);
            break;
        case gs_value_type_u16:
            required_size += sizeof(atomic_u16_t);
            break;
        case gs_value_type_u32:
            required_size += sizeof(atomic_u32_t);
            break;
        case gs_value_type_u64:
            required_size += sizeof(atomic_u64_t);
            break;
        case gs_value_type_s8:
            required_size += sizeof(atomic_s8_t);
            break;
        case gs_value_type_s16:
            required_size += sizeof(atomic_s16_t);
            break;
        case gs_value_type_s32:
            required_size += sizeof(atomic_s32_t);
            break;
        case gs_value_type_s64:
            required_size += sizeof(atomic_s64_t);
            break;
        case gs_value_type_float32:
            required_size += sizeof(atomic_float32_t);
            break;
        case gs_value_type_float64:
            required_size += sizeof(atomic_float64_t);
            break;
        case gs_value_type_string:
            required_size += sizeof(c_string_t);
            break;
        case gs_value_type_boolean:
            required_size += sizeof(atomic_boolean_t);
            break;
        case gs_value_type_null:
            break;
        case gs_value_type_object:
        case gs_value_type_array:
            required_size += sizeof(gs_base_relative_ptr_t);
            break;
        default: panic("unknown value type: '%d'", type);
    }
    return required_size;
}

GS_DECLARE(gs_status_t) gs_value_new_generic(gs_base_relative_ptr_t *result, BYTE *dst, gs_value_type_e type,
                                             gs_base_relative_ptr_t parent, gs_value_parent_type_e parent_type,
                                             const void *data)
{
    return 0;
}

GS_DECLARE(gs_status_t) gs_value_get_type(gs_value_type_e *type, const BYTE *ptr)
{
    return 0;
}

GS_DECLARE(gs_status_t) gs_value_get_parent(gs_base_relative_ptr_t parent, const BYTE *ptr)
{
    return 0;
}

GS_DECLARE(gs_status_t) gs_value_get_parent_type(gs_value_parent_type_e *type, const BYTE *ptr)
{
    return 0;
}

GS_DECLARE(void *) gs_value_read_generic(const BYTE *ptr)
{
    return 0;
}

GS_DECLARE(void *) gs_value_write_generic(const BYTE *ptr, const void *data)
{
    return 0;
}