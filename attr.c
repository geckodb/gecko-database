#include <attr.h>
#include <containers/vector.h>
#include <field.h>
#include <frag.h>

#define DEFINE_ATTRIBUTE_CREATE(type_name,internal_type)                                                               \
attr_id_t gs_attr_create_##type_name(const char *name, schema_t *schema) {                                             \
    return gs_attr_create(name, internal_type, 1, schema);                                                             \
}

#define DEFINE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                         \
attr_id_t gs_attr_create_##type_name(const char *name, size_t length, schema_t *schema) {                              \
    return gs_attr_create(name, internal_type, length, schema);                                                        \
}

attr_id_t _attr_create(const char *name, enum field_type data_type, size_t data_type_rep, ATTR_FLAGS attr_flags,
                     attr_t *foreign_key_to, schema_t *schema)
{
    panic_if((name == NULL || schema == NULL || strlen(name) + 1 > ATTR_NAME_MAXLEN), BADARG,
             "null pointer or attribute name limit exceeded");

    attr_t attr = {
            .id     = schema->attr->num_elements,
            .type     = data_type,
            .type_rep = data_type_rep,
            .flags    = attr_flags,
            .str_format_mlen   = 0,
            .foreign_id = (foreign_key_to != NULL? foreign_key_to->id : -1) // TODO: this
    };

    strcpy(attr.name, name);
    memset(attr.checksum, 0, MD5_DIGEST_LENGTH);
    vector_add(schema->attr, 1, &attr);
    return attr.id;
}

const char *gs_attr_get_name(const struct attr_t *attr)
{
    assert (attr);
    return attr->name;
}

size_t gs_attr_get_str_format_max_len(attr_t *attr)
{
    assert (attr);
    return attr->str_format_mlen;
}

enum field_type gs_attr_get_type(const attr_t *attr)
{
    assert (attr);
    return attr->type;
}

attr_t *gs_attr_cpy(const attr_t *template, schema_t *new_owner)
{
    assert (template);
    assert(new_owner);
//    _attr_create(template->name, template->type, template->type_rep, template->flags, template.)
    return NULL; // TODO
}

attr_id_t gs_attr_create(const char *name, enum field_type data_type, size_t data_type_rep, schema_t *schema)
{
    return _attr_create(name, data_type, data_type_rep,
                        (ATTR_FLAGS) { .autoinc = 0, .foreign = 0, .nullable = 0, .primary = 0, .unique = 0 },
                        NULL, schema);
}

bool gs_attr_isstring(const attr_t *attr)
{
    return (attr == NULL ? false : (attr->type == FT_CHAR));
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

