#include <schema.h>
#include <fragment.h>
#include <attr.h>

ATTR_ID _attr_create(const char *name, enum field_type data_type, size_t data_type_rep, ATTR_FLAGS attr_flags,
                     ATTR *foreign_key_to, SCHEMA *schema)
{
    if (name == NULL || schema == NULL ||strlen(name) + 1 > _ATTR_NUMMAX)
        return -1;

    ATTR attr = {
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

ATTR_ID _attr_default(const char *name, enum field_type data_type, size_t data_type_rep, SCHEMA *schema)
{
    return _attr_create(name, data_type, data_type_rep,
                                 (ATTR_FLAGS) { .autoinc = 0, .foreign = 0, .nullable = 0, .primary = 0, .unique = 0 },
                                 NULL, schema);
}

#define DEFINE_ATTRIBUTE_CREATE(type_name,internal_type)                                                               \
ATTR_ID gs_attr_create_##type_name(const char *name, SCHEMA *schema) {                                                 \
    return _attr_default(name, internal_type, 1, schema);                                                              \
}

#define DEFINE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                         \
ATTR_ID gs_attr_create_##type_name(const char *name, size_t length, SCHEMA *schema) {                                  \
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
void *gs_insert_##type_name(void *dst, SCHEMA *schema, ATTR_ID attr_id, const c_type *src)                           \
{                                                                                                                      \
    assert (attr_id < schema->attr->num_elements);                                                                     \
    ATTR *attr = vector_at(schema->attr, attr_id);                                                                     \
    assert (internal_type == attr->type);                                                                              \
    attr->str_format_mlen = max(attr->str_format_mlen, gs_tuplet_printlen(attr, src));                                 \
    size_t field_size = get_field_size(attr, 0);                                                                       \
    memcpy(dst, src, get_field_size(attr, 0));                                                                         \
    return dst + field_size;                                                                                           \
}

#define DEFINE_ARRAY_FIELD_INSERT(type_name, c_type, internal_type)                                                    \
void *gs_insert_##type_name(void *dst, SCHEMA *schema, ATTR_ID attr_id, const c_type *src)                           \
{                                                                                                                      \
    assert (attr_id < schema->attr->num_elements);                                                                     \
    ATTR *attr = vector_at(schema->attr, attr_id);                                                                     \
    assert (internal_type == attr->type);                                                                              \
    attr->str_format_mlen = max(attr->str_format_mlen, gs_tuplet_printlen(attr, src));                                 \
    assert (strlen(src) < attr->type_rep);                                                                             \
    strcpy(dst, src);                                                                                                  \
    return dst + get_field_size(attr, 0);                                                                              \
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


size_t gs_sizeof(enum field_type type)
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

size_t get_field_size(const ATTR *attr, size_t attr_idx)
{
    return attr[attr_idx].type_rep * gs_sizeof(attr[attr_idx].type);
}

size_t get_tuple_size(SCHEMA *schema)
{
    size_t total = 0;
    for (size_t attr_idx = 0; attr_idx < schema->attr->num_elements; attr_idx++) {
        total += get_field_size((ATTR *) schema->attr->data, attr_idx);
    }
    return total;
}

SCHEMA *gs_schema_create()
{
    SCHEMA *result = malloc(sizeof(SCHEMA));
    result->attr = vector_create(sizeof(ATTR), 100);
    return result;
}

void *gs_update(void *dst, SCHEMA *frag, ATTR_ID attr_id, void *src)
{
    assert (dst && frag && src);



    switch (gs_schema_attr_by_id(attr_id)->type) {
        case FT_BOOL:    return gs_insert_bool(dst, frag, attr_id,    (const BOOL *) src);
        case FT_INT8:    return gs_insert_int8(dst, frag, attr_id,    (const INT8 *) src);
        case FT_INT16:   return gs_insert_int16(dst, frag, attr_id,   (const INT16 *) src);
        case FT_INT32:   return gs_insert_int32(dst, frag, attr_id,   (const INT32 *) src);
        case FT_INT64:   return gs_insert_int64(dst, frag, attr_id,   (const INT64 *) src);
        case FT_UINT8:   return gs_insert_uint8(dst, frag, attr_id,   (const UINT8 *) src);
        case FT_UINT16:  return gs_insert_uint16(dst, frag, attr_id,  (const UINT16 *) src);
        case FT_UINT32:  return gs_insert_uint32(dst, frag, attr_id,  (const UINT32 *) src);
        case FT_UINT64:  return gs_insert_uint64(dst, frag, attr_id,  (const UINT64 *) src);
        case FT_FLOAT32: return gs_insert_float32(dst, frag, attr_id, (const FLOAT32 *) src);
        case FT_FLOAT64: return gs_insert_float64(dst, frag, attr_id, (const FLOAT64 *) src);
        case FT_CHAR:    return gs_insert_string(dst, frag, attr_id,  (const CHAR *) src);
    }
}

FRAGMENT *gs_fragment_alloc(SCHEMA *frag, size_t num_tuplets, enum tuplet_format format)
{
    FRAGMENT *fragment = malloc(sizeof(FRAGMENT));
    *fragment = (FRAGMENT) {
        .format = format,
        .ntuplets = num_tuplets,
        .tuplet_data = malloc (get_tuple_size(frag) * num_tuplets)
    };
    return fragment;
}

void gs_fragment_free(FRAGMENT *frag)
{
    assert(frag);
    assert(frag->tuplet_data);
    free (frag->tuplet_data);
    free (frag);
}

const char *gs_type_str(enum field_type t)
{
    switch (t) {
        case FT_BOOL:    return "bool";
        case FT_INT8:    return "int8";
        case FT_INT16:   return "int16";
        case FT_INT32:   return "int32";
        case FT_INT64:   return "int64";
        case FT_UINT8:   return "uint8";
        case FT_UINT16:  return "uint16";
        case FT_UINT32:  return "uint32";
        case FT_UINT64:  return "uint64";
        case FT_FLOAT32: return "float32";
        case FT_FLOAT64: return "float64";
        case FT_CHAR:  return "string";
        default: return "(unknown)";
    }
}

size_t gs_tuplet_printlen(const ATTR *attr, const void *field_data)
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

void gs_checksum_nsm(SCHEMA *tab, const void *tuplets, size_t ntuplets)
{
    size_t num_attr = tab->attr->num_elements;
    ATTR *attr = (ATTR *) tab->attr->data;
    size_t tuple_size = get_tuple_size(tab);
    CHECKSUM_CONTEXT column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        const void *cursor = tuplets;
        size_t field_offs = get_tuple_size(tab);
        size_t field_size = get_field_size(attr, attr_idx);
        begin_checksum(&column_checksum);
        for (size_t tuple_idx = 0; tuple_idx < num_attr; tuple_idx++) {
            update_checksum(&column_checksum, cursor + field_offs, cursor + field_offs + field_size);
            cursor += tuple_size;
        }
        end_checksum(attr[attr_idx].checksum, &column_checksum);
    }
}

void gs_checksum_dms(SCHEMA *tab, const void *tuplets, size_t ntuplets)
{
    size_t num_attr = tab->attr->num_elements;
    ATTR *attr = (ATTR *) tab->attr->data;
    CHECKSUM_CONTEXT column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t field_size = get_field_size(attr, attr_idx);
        size_t chunk_size = ntuplets * field_size;
        begin_checksum(&column_checksum);
        update_checksum(&column_checksum, tuplets, tuplets + chunk_size);
        end_checksum(attr[attr_idx].checksum, &column_checksum);
        tuplets += chunk_size;
    }
}

void begin_checksum(CHECKSUM_CONTEXT *context)
{
    MD5_Init (context);
}

void update_checksum(CHECKSUM_CONTEXT *context, const void *begin, const void *end) {
    MD5_Update (context, begin, (end - begin));
}

void end_checksum(unsigned char *checksum_out, CHECKSUM_CONTEXT *context)
{
    MD5_Final (checksum_out,context);
}

