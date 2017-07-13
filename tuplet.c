#include <tuplet.h>
#include <field.h>
#include <schema.h>
//
//#define DEFINE_TUPLET_INSERT(type_name, c_type, internal_type)                                                         \
//void *gs_insert_##type_name(void *dst, schema_t *schema, attr_id_t attr_id, const c_type *src)                         \
//{                                                                                                                      \
//    assert (attr_id < schema->attr->num_elements);                                                                     \
//    attr_t *attr = vector_at(schema->attr, attr_id);                                                                   \
//    assert (internal_type == attr->type);                                                                              \
//    attr->str_format_mlen = max(attr->str_format_mlen, gs_tuplet_printlen(attr, src));                                 \
//    panic(NOTIMPLEMENTED, "this")    \
//    size_t field_size = 0;/*gs_field_size(attr, 0);*/                                                                  \
//   /* memcpy(dst, src, get_field_size(attr, 0));   */                                                                  \
//    return dst + field_size;                                                                                           \
//}
//
//#define DEFINE_ARRAY_FIELD_INSERT(type_name, c_type, internal_type)                                                    \
//void *gs_insert_##type_name(void *dst, schema_t *schema, attr_id_t attr_id, const c_type *src)                         \
//{                                                                                                                      \
//    assert (attr_id < schema->attr->num_elements);                                                                     \
//    attr_t *attr = vector_at(schema->attr, attr_id);                                                                   \
//    assert (internal_type == attr->type);                                                                              \
//    attr->str_format_mlen = max(attr->str_format_mlen, gs_tuplet_printlen(attr, src));                                 \
//    assert (strlen(src) < attr->type_rep);                                                                             \
//    strcpy(dst, src);                                                                                                  \
//    /*return dst + get_field_size(attr, 0); */                                                                         \
//    panic(NOTIMPLEMENTED, "this")   \
//return 0;   \
//}
//
//DEFINE_TUPLET_INSERT(bool, bool, FT_BOOL)
//DEFINE_TUPLET_INSERT(int8, int8_t, FT_INT8)
//DEFINE_TUPLET_INSERT(int16, int16_t, FT_INT16)
//DEFINE_TUPLET_INSERT(int32, int32_t, FT_INT32)
//DEFINE_TUPLET_INSERT(int64, int64_t, FT_INT64)
//DEFINE_TUPLET_INSERT(uint8, uint8_t, FT_UINT8)
//DEFINE_TUPLET_INSERT(uint16, uint16_t, FT_UINT16)
//DEFINE_TUPLET_INSERT(uint32, uint32_t, FT_UINT32)
//DEFINE_TUPLET_INSERT(uint64, uint64_t, FT_UINT64)
//DEFINE_TUPLET_INSERT(float32, float, FT_FLOAT32)
//DEFINE_TUPLET_INSERT(float64, double, FT_FLOAT64)
//DEFINE_ARRAY_FIELD_INSERT(string, char, FT_CHAR)

tuplet_t *gs_tuplet_open(struct fragment_t *frag)
{
    if (frag->ntuplets > 0) {
        require_non_null(frag);
        require_non_null(frag->_open);
        tuplet_t *result = frag->_open(frag);
        panic_if((result == NULL), UNEXPECTED, "fragment_t::open return NULL");
        return result;
    } else return NULL;
}

bool gs_tuplet_next(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_next);
    return tuplet->_next(tuplet);
}

tuplet_t *gs_tuplet_rewind(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_close);
    fragment_t *frag = tuplet->fragment;
    tuplet->_close(tuplet);
    return gs_tuplet_open(frag);
}

void gs_tuplet_set_null(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_set_null);
    tuplet->_set_null(tuplet);
}

bool gs_tuplet_is_null(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_is_null);
    return (tuplet->_is_null(tuplet));
}

void gs_tuplet_close(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_close);
    tuplet->_close(tuplet);
}

size_t gs_tuplet_size(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->fragment);
    return tuplet->fragment->tuplet_size;
}

void *gs_update(void *dst, schema_t *frag, attr_id_t attr_id, void *src)
{
    panic(NOTIMPLEMENTED, to_string(gs_update))
    return NULL;
}

size_t gs_tuplet_printlen(const attr_t *attr, const void *field_data)
{
    char buffer[2048];

    switch (attr->type) {
        case FT_BOOL:
            return (*((bool *) field_data) == true? strlen("true") : strlen("false"));
        case FT_INT8:
            sprintf(buffer, "%d", *(INT8 *) field_data);
            break;
        case FT_INT16:
            sprintf(buffer, "%d", *(INT16 *) field_data);
            break;
        case FT_INT32:
            sprintf(buffer, "%d", *(INT32 *) field_data);
            break;
        case FT_INT64:
            sprintf(buffer, "%lld", *(INT64 *) field_data);
            break;
        case FT_UINT8:
            sprintf(buffer, "%u", *(UINT8 *) field_data);
            break;
        case FT_UINT16:
            sprintf(buffer, "%u", *(UINT16 *) field_data);
            break;
        case FT_UINT32:
            sprintf(buffer, "%u", *(UINT32 *) field_data);
            break;
        case FT_UINT64:
            sprintf(buffer, "%llu", *(UINT64 *) field_data);
            break;
        case FT_FLOAT32:
            sprintf(buffer, "%f", *(FLOAT32 *) field_data);
            break;
        case FT_FLOAT64:
            sprintf(buffer, "%f", *(FLOAT64 *) field_data);
            break;
        case FT_CHAR:
            return strlen((CHAR *) field_data);
            break;
        default:
            perror("Unknown type");
            abort();
    }
    return strlen(buffer);
}

size_t gs_tuplet_size_by_schema(const schema_t *schema)
{
    size_t total = 0;
    for (size_t attr_idx = 0; attr_idx < schema->attr->num_elements; attr_idx++) {
        attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
        total += gs_attr_total_size(attr);
    }
    return total;
}
