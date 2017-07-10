#include <attr.h>
#include <containers/vector.h>
#include <field.h>
#include <frag.h>

attr_id_t _attr_create(const char *name, enum field_type data_type, size_t data_type_rep, ATTR_FLAGS attr_flags,
                     attr_t *foreign_key_to, schema_t *schema)
{
    if (name == NULL || schema == NULL ||strlen(name) + 1 > _ATTR_NUMMAX)
        return -1;

    attr_t attr = {
            .id     = schema->attr->num_elements,
            .type     = data_type,
            .type_rep = data_type_rep,
            .flags    = attr_flags,
            .str_format_mlen   = 0,
            .foreign_id = (foreign_key_to != NULL? foreign_key_to->id : -1)
    };

    strcpy(attr.name, name);
    memset(attr.checksum, 0, MD5_DIGEST_LENGTH);
    vector_add(schema->attr, 1, &attr);
    return attr.id;
}

attr_id_t _attr_default(const char *name, enum field_type data_type, size_t data_type_rep, schema_t *schema)
{
    return _attr_create(name, data_type, data_type_rep,
                        (ATTR_FLAGS) { .autoinc = 0, .foreign = 0, .nullable = 0, .primary = 0, .unique = 0 },
                        NULL, schema);
}

#define DEFINE_ATTRIBUTE_CREATE(type_name,internal_type)                                                               \
attr_id_t gs_attr_create_##type_name(const char *name, schema_t *schema) {                                             \
    return _attr_default(name, internal_type, 1, schema);                                                              \
}

#define DEFINE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                         \
attr_id_t gs_attr_create_##type_name(const char *name, size_t length, schema_t *schema) {                              \
    return _attr_default(name, internal_type, length, schema);                                                         \
}

DEFINE_ATTRIBUTE_CREATE(bool, FT_BOOL)
DEFINE_ATTRIBUTE_CREATE(int8, FT_INT8)
DEFINE_ATTRIBUTE_CREATE(int16, FT_INT16)
DEFINE_ATTRIBUTE_CREATE(int32, FT_INT32)
DEFINE_ATTRIBUTE_CREATE(int64, FT_INT64)
DEFINE_ATTRIBUTE_CREATE(uint8, FT_UINT8)
DEFINE_ATTRIBUTE_CREATE(uint16, FT_UINT16)
DEFINE_ATTRIBUTE_CREATE(uint32, FT_UINT32)
DEFINE_ATTRIBUTE_CREATE(uint64, FT_UINT64)
DEFINE_ATTRIBUTE_CREATE(float32, FT_FLOAT32)
DEFINE_ATTRIBUTE_CREATE(float64, FT_FLOAT64)
DEFINE_ATTRIBUTE_ARRAY_CREATE(string, FT_CHAR)

#define DEFINE_TUPLET_INSERT(type_name, c_type, internal_type)                                                         \
void *gs_insert_##type_name(void *dst, schema_t *schema, attr_id_t attr_id, const c_type *src)                         \
{                                                                                                                      \
    assert (attr_id < schema->attr->num_elements);                                                                     \
    attr_t *attr = vector_at(schema->attr, attr_id);                                                                   \
    assert (internal_type == attr->type);                                                                              \
    attr->str_format_mlen = max(attr->str_format_mlen, gs_tuplet_printlen(attr, src));                                 \
    panic(NOTIMPLEMENTED, "this")    \
    size_t field_size = 0;/*gs_field_size(attr, 0);*/                                                                        \
   /* memcpy(dst, src, get_field_size(attr, 0));   */                                                                      \
    return dst + field_size;                                                                                           \
}

#define DEFINE_ARRAY_FIELD_INSERT(type_name, c_type, internal_type)                                                    \
void *gs_insert_##type_name(void *dst, schema_t *schema, attr_id_t attr_id, const c_type *src)                         \
{                                                                                                                      \
    assert (attr_id < schema->attr->num_elements);                                                                     \
    attr_t *attr = vector_at(schema->attr, attr_id);                                                                   \
    assert (internal_type == attr->type);                                                                              \
    attr->str_format_mlen = max(attr->str_format_mlen, gs_tuplet_printlen(attr, src));                                 \
    assert (strlen(src) < attr->type_rep);                                                                             \
    strcpy(dst, src);                                                                                                  \
    /*return dst + get_field_size(attr, 0); */                                                                             \
    panic(NOTIMPLEMENTED, "this")   \
return 0;   \
}

DEFINE_TUPLET_INSERT(bool, bool, FT_BOOL)
DEFINE_TUPLET_INSERT(int8, int8_t, FT_INT8)
DEFINE_TUPLET_INSERT(int16, int16_t, FT_INT16)
DEFINE_TUPLET_INSERT(int32, int32_t, FT_INT32)
DEFINE_TUPLET_INSERT(int64, int64_t, FT_INT64)
DEFINE_TUPLET_INSERT(uint8, uint8_t, FT_UINT8)
DEFINE_TUPLET_INSERT(uint16, uint16_t, FT_UINT16)
DEFINE_TUPLET_INSERT(uint32, uint32_t, FT_UINT32)
DEFINE_TUPLET_INSERT(uint64, uint64_t, FT_UINT64)
DEFINE_TUPLET_INSERT(float32, float, FT_FLOAT32)
DEFINE_TUPLET_INSERT(float64, double, FT_FLOAT64)
DEFINE_ARRAY_FIELD_INSERT(string, char, FT_CHAR)

