#include <field_type.h>

size_t gs_field_type_sizeof(enum field_type type)
{
    switch (type) {
        case FT_CHAR:
            return sizeof(CHAR);
        case FT_BOOL:
            return sizeof(BOOL);
        case FT_INT8:
            return sizeof(INT8);
        case FT_UINT8:
            return sizeof(UINT8);
        case FT_INT16:
            return sizeof(INT16);
        case FT_UINT16:
            return sizeof(UINT16);
        case FT_INT32:
            return sizeof(INT32);
        case FT_UINT32:
            return sizeof(UINT32);
        case FT_FLOAT32:
            return sizeof(FLOAT32);
        case FT_INT64:
            return sizeof(INT64);
        case FT_UINT64:
            return sizeof(UINT64);
        case FT_FLOAT64:
            return sizeof(FLOAT64);
        default:
            perror ("Unknown type");
            abort();
    }
}