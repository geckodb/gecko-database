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
        /* internal */
        case FT_STRPTR:
            return sizeof(STRPTR);
        case FT_ATTRID:
            return sizeof(ATTRID);
        default:
            perror ("Unknown type");
            abort();
    }
}

const char *gs_field_type_str(enum field_type type)
{
    switch (type) {
        case FT_CHAR:
            return "char";
        case FT_BOOL:
            return "bool";
        case FT_INT8:
            return "s8";
        case FT_UINT8:
            return "u8";
        case FT_INT16:
            return "s16";
        case FT_UINT16:
            return "u16";
        case FT_INT32:
            return "s32";
        case FT_UINT32:
            return "u32";
        case FT_FLOAT32:
            return "float32";
        case FT_INT64:
            return "s64";
        case FT_UINT64:
            return "u64";
        case FT_FLOAT64:
            return "float64";
            /* internal */
        case FT_STRPTR:
            return "string";
        case FT_ATTRID:
            return "attr id";
        default:
            perror ("Unknown type");
            abort();
    }
}