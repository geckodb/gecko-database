#include <tableimg.h>
#include <stddef.h>
#include <defs.h>
#include <containers/vector.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <openssl/md5.h>

typedef MD5_CTX checksum_context_t;

void begin_checksum(checksum_context_t *conext);
void update_checksum(checksum_context_t *context, const void *begin, const void *end);
void end_checksum(unsigned char *checksum_out, checksum_context_t *context);

void calc_column_checksum_nsm(
        timg_schema_t   schema,
        const void     *tuple_data,
        size_t          tuple_num
);

void calc_column_checksum_dsm(
        timg_schema_t   schema,
        const void     *tuple_data,
        size_t          tuple_num
);

size_t get_data_type_size(timg_data_type_t type)
{
    switch (type) {
        case TIMG_DATATYPE_STRING: case TIMG_DATATYPE_BOOL: case TIMG_DATATYPE_INT8: case TIMG_DATATYPE_UINT8:
            return 8;
        case TIMG_DATATYPE_INT16: case TIMG_DATATYPE_UINT16:
            return 16;
        case TIMG_DATATYPE_INT32: case TIMG_DATATYPE_UINT32: case TIMG_DATATYPE_FLOAT32:
            return 32;
        case TIMG_DATATYPE_INT64: case TIMG_DATATYPE_UINT64: case TIMG_DATATYPE_FLOAT64:
            return 64;
        default:
            perror ("Unknown type");
            abort();
    }
}

size_t get_field_size(
        const timg_attr_t *attr,
        size_t             attr_idx)
{
    return attr[attr_idx].data_type_rep * get_data_type_size(attr[attr_idx].data_type);
}

size_t get_tuple_size(
        timg_schema_t schema)
{
    size_t total = 0;
    for (size_t attr_idx = 0; attr_idx < schema->attributes->num_elements; attr_idx++) {
        total += get_field_size((timg_attr_t *) schema->attributes->data, attr_idx);
    }
    return total;
}

timg_schema_t timg_schema_create()
{
    timg_schema_t result = malloc(sizeof(timg_schema_t));
    result->attributes = vector_create(sizeof(timg_attr_t), 100);
    return result;
}

timg_attr_id_t timg_attribute_create(
        const char       *name,
        timg_data_type_t  data_type,
        size_t            data_type_rep,
        attr_flags_t      attr_flags,
        timg_attr_t      *foreign_key_to,
        timg_schema_t     schema)
{
    if (name == NULL || schema == NULL ||strlen(name) + 1 > TIMG_ATTRIBUTE_NAME_MAX)
        return -1;

    timg_attr_t attr;

    attr.unique_id     = schema->attributes->num_elements;
    attr.data_type     = data_type;
    attr.data_type_rep = data_type_rep;
    attr.attr_flags    = attr_flags;
    if (foreign_key_to) {
        attr.foreign_unique_id = foreign_key_to->unique_id;
    }
    strcpy(attr.name, name);
    memset(attr.checksum, 0, MD5_DIGEST_LENGTH);

    vector_add(schema->attributes, 1, &attr);

    return attr.unique_id;
}

timg_attr_id_t timg_attribute_default(
        const char       *name,
        timg_data_type_t  data_type,
        size_t            data_type_rep,
        timg_schema_t     schema)
{
    return timg_attribute_create(name, data_type, data_type_rep,
                                 (attr_flags_t) { .is_autoinc = 0, .is_foreign_key = 0, .is_nullable = 0,
                                         .is_primary_key = 0, .is_unique = 0 },
                                 NULL, schema);
}

#define DEFINE_ATTRIBUTE_CREATE(type_name,internal_type)                       \
timg_attr_id_t timg_attribute_create_##type_name(                               \
    const char       *name,                                                     \
    timg_schema_t     schema                                                    \
) {                                                                             \
    return timg_attribute_default(name, internal_type, 1, schema);              \
}

#define DEFINE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                 \
timg_attr_id_t timg_attribute_create_##type_name(                               \
    const char       *name,                                                     \
    size_t            length,                                                   \
    timg_schema_t     schema                                                    \
) {                                                                             \
    return timg_attribute_default(name, internal_type, length, schema);         \
}

DEFINE_ATTRIBUTE_CREATE(bool, TIMG_DATATYPE_BOOL)
DEFINE_ATTRIBUTE_CREATE(int8, TIMG_DATATYPE_INT8)
DEFINE_ATTRIBUTE_CREATE(int16, TIMG_DATATYPE_INT16)
DEFINE_ATTRIBUTE_CREATE(int32, TIMG_DATATYPE_INT32)
DEFINE_ATTRIBUTE_CREATE(int64, TIMG_DATATYPE_INT64)
DEFINE_ATTRIBUTE_CREATE(uint8, TIMG_DATATYPE_UINT8)
DEFINE_ATTRIBUTE_CREATE(uint16, TIMG_DATATYPE_UINT16)
DEFINE_ATTRIBUTE_CREATE(uint32, TIMG_DATATYPE_UINT32)
DEFINE_ATTRIBUTE_CREATE(uint64, TIMG_DATATYPE_UINT64)
DEFINE_ATTRIBUTE_CREATE(float32, TIMG_DATATYPE_FLOAT32)
DEFINE_ATTRIBUTE_CREATE(float64, TIMG_DATATYPE_FLOAT64)
DEFINE_ATTRIBUTE_ARRAY_CREATE(string, TIMG_DATATYPE_STRING)

void *timg_table_create(
        timg_schema_t schema,
        size_t num_tuples)
{
    return malloc (get_tuple_size(schema) * num_tuples);
}

size_t tableimg_rawsize(
        timg_header_t *header)
{
    size_t total = 0;
    for (size_t i = 0; i < header->num_attributes; i++) {
        total += get_field_size(&header->attributes[0], i);
    }
    return (total * header->num_tuples);
}

const char *tableimg_datatype_str(timg_data_type_t t)
{
    switch (t) {
        case TIMG_DATATYPE_BOOL:    return "bool";
        case TIMG_DATATYPE_INT8:    return "int8";
        case TIMG_DATATYPE_INT16:   return "int16";
        case TIMG_DATATYPE_INT32:   return "int32";
        case TIMG_DATATYPE_INT64:   return "int64";
        case TIMG_DATATYPE_UINT8:   return "uint8";
        case TIMG_DATATYPE_UINT16:  return "uint16";
        case TIMG_DATATYPE_UINT32:  return "uint32";
        case TIMG_DATATYPE_UINT64:  return "uint64";
        case TIMG_DATATYPE_FLOAT32: return "float32";
        case TIMG_DATATYPE_FLOAT64: return "float64";
        case TIMG_DATATYPE_STRING:  return "string";
        default: return "(unknown)";
    }
}

void timg_table_free(void *data)
{
    if (data != NULL)
        free (data);
}

#define DEFINE_FIELD_WRITE(type_name, c_type, internal_type)                                               \
void *timg_field_write_##type_name(void *dst, const timg_schema_t schema, timg_attr_id_t attr_id, const c_type *src)         \
{                                                                                               \
    assert (attr_id < schema->attributes->num_elements); \
    const timg_attr_t *attr = vector_at(schema->attributes, attr_id); \
    assert (internal_type == attr->data_type);                                                  \
    size_t field_size = get_field_size(attr, 0);                                                \
    memcpy(dst, src, get_field_size(attr, 0));                                                  \
    return dst + field_size;                                                                    \
}

#define DEFINE_ARRAY_FIELD_WRITE(type_name, c_type, internal_type)                                               \
void *timg_field_write_##type_name(void *dst, const timg_schema_t schema, timg_attr_id_t attr_id, const c_type *src)         \
{                                                                                               \
    assert (attr_id < schema->attributes->num_elements); \
    const timg_attr_t *attr = vector_at(schema->attributes, attr_id); \
    assert (internal_type == attr->data_type);                                                  \
    assert (strlen(src) < attr->data_type_rep);                                                  \
    strcpy(dst, src);                                                  \
    return dst + get_field_size(attr, 0);                                                                    \
}

DEFINE_FIELD_WRITE(bool, bool, TIMG_DATATYPE_BOOL)
DEFINE_FIELD_WRITE(int8, int8_t, TIMG_DATATYPE_INT8)
DEFINE_FIELD_WRITE(int16, int16_t, TIMG_DATATYPE_INT16)
DEFINE_FIELD_WRITE(int32, int32_t, TIMG_DATATYPE_INT32)
DEFINE_FIELD_WRITE(int64, int64_t, TIMG_DATATYPE_INT64)
DEFINE_FIELD_WRITE(uint8, uint8_t, TIMG_DATATYPE_UINT8)
DEFINE_FIELD_WRITE(uint16, uint16_t, TIMG_DATATYPE_UINT16)
DEFINE_FIELD_WRITE(uint32, uint32_t, TIMG_DATATYPE_UINT32)
DEFINE_FIELD_WRITE(uint64, uint64_t, TIMG_DATATYPE_UINT64)
DEFINE_FIELD_WRITE(float32, float, TIMG_DATATYPE_FLOAT32)
DEFINE_FIELD_WRITE(float64, double, TIMG_DATATYPE_FLOAT64)
DEFINE_ARRAY_FIELD_WRITE(string, char, TIMG_DATATYPE_STRING)


timg_error_t tableimg_fwrite(
        FILE *file,
        timg_version_t version,
        const char *database_name,
        const char *table_name,
        const char *table_spec_ref,
        const char *comment,
        timg_schema_t schema,
        const void *tuple_data,
        size_t num_tuples,
        timg_format_t format_in,
        timg_format_t format_out)
{
    if ((file == NULL ) || (database_name == NULL) || (table_name == NULL) || (table_spec_ref == NULL) ||
        (schema == NULL) || (tuple_data == NULL) || schema->attributes->num_elements == 0)
        return TIMG_ERR_ILLEGALARG;

    int file_desc = fileno(file);
    fcntl(file_desc, F_GETFL);
    if (fcntl(file_desc, F_GETFL) == -1 && errno == EBADF)
        return TIMG_ERR_BADFILE;

    if (strlen(database_name) + 1 > TIMG_DATABASE_NAME_MAX)
        return TIMG_ERR_DBNAME_TOOLONG;

    if (strlen(table_name) + 1 > TIMG_TABLE_NAME_MAX)
        return TIMG_ERR_TABNAME_TOOLONG;

    if (strlen(table_spec_ref) + 1 > TIMG_TABLE_SPEC_REF_MAX)
        return TIMG_ERR_TABSPEC_TOOLONG;

    if (schema->attributes->num_elements > TIMG_ATTRIBUTE_NUM_MAX)
        return TIMG_ERR_TOMANYATTRIBUTES;

    if (strlen(comment) + 1 > TIMG_COMMENT_MAX)
        return TIMG_ERR_COMMENT_TOOLONG;

    rewind(file);
    fseek(file, 0L, SEEK_SET);

    size_t tuple_size = get_tuple_size(schema);
    size_t field_offs = 0;
    size_t field_size = 0;
    const void   *cursor    = NULL;

    size_t num_attr = schema->attributes->num_elements;
    timg_attr_t *attr = (timg_attr_t *) schema->attributes->data;

    switch (format_in) {
        case SF_NSM:
            calc_column_checksum_nsm(schema, tuple_data, num_tuples);
            switch (format_out) {
                case SF_NSM:
                    fwrite(tuple_data, tuple_size, num_tuples, file);
                    break;
                case SF_DSM:
                    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
                        cursor     = tuple_data;
                        field_offs = get_tuple_size(schema);
                        field_size = get_field_size(attr, attr_idx);
                        for (size_t tuple_idx = 0; tuple_idx < num_tuples; tuple_idx++) {
                            fwrite(cursor + field_offs, field_size, 1, file);
                            cursor += tuple_size;
                        }
                    }
                    break;
                default:
                    return TIMG_ERR_BADSERIALFORMAT;
            }
            break;
        case SF_DSM:
            calc_column_checksum_dsm(schema, tuple_data, num_tuples);
            switch (format_out) {
                case SF_NSM:
                    cursor     = tuple_data;
                    for (size_t tuple_idx = 0; tuple_idx < num_attr; tuple_idx++) {
                        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
                            field_offs = 0;

                            for (size_t prev_attr_idx = 0; prev_attr_idx < attr_idx; prev_attr_idx++) {
                                field_offs = num_tuples * get_field_size(attr, attr_idx);
                            }
                            field_size = get_field_size(attr, attr_idx);
                            field_offs += tuple_idx * field_size;
                            fwrite(cursor + field_offs, field_size, 1, file);
                        }
                    }
                    break;
                case SF_DSM:
                    fwrite(tuple_data, tuple_size, num_tuples, file);
                    break;
                default:
                    return TIMG_ERR_BADSERIALFORMAT;
            }
            break;
        default:
            return TIMG_ERR_BADSERIALFORMAT;
    }

    fseek(file, 0L, SEEK_SET);

    timg_header_t header = {
            .magic                  = TIMG_MAGIC_WORD,
            .format_version         = version,

            .flags = {
                    .serial_format_type = (format_out == SF_NSM ? TIMG_FORMAT_NSM : TIMG_SERIALIZATION_FORMAT_DSM)
            },

            .num_tuples        = num_tuples,
            .timestamp_created = (uint64_t) time(NULL),
    };
    checksum_context_t raw_data_checksum;
    begin_checksum(&raw_data_checksum);
    update_checksum(&raw_data_checksum, tuple_data, tuple_data + tuple_size * num_tuples);
    end_checksum(header.raw_table_data_checksum, &raw_data_checksum);

    strcpy(header.database_name, database_name);
    strcpy(header.table_name, table_name);
    strcpy(header.table_spec_ref, table_spec_ref);
    strcpy(header.comment, comment);

    header.num_attributes = schema->attributes->num_elements;
    memcpy(header.attributes, schema->attributes->data, sizeof(timg_attr_t) * schema->attributes->num_elements);

    fwrite(&header, sizeof(timg_header_t), 1, file);

    return TIMG_ERR_OK;
}

void calc_column_checksum_nsm(
    timg_schema_t   schema,
    const void     *tuple_data,
    size_t          tuple_num)
{
    size_t             num_attr   = schema->attributes->num_elements;
    timg_attr_t       *attr       = (timg_attr_t *) schema->attributes->data;
    size_t             tuple_size = get_tuple_size(schema);
    checksum_context_t column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        const void *cursor     = tuple_data;
        size_t      field_offs = get_tuple_size(schema);
        size_t      field_size = get_field_size(attr, attr_idx);
        begin_checksum(&column_checksum);
        for (size_t tuple_idx = 0; tuple_idx < num_attr; tuple_idx++) {
            update_checksum(&column_checksum, cursor + field_offs, cursor + field_offs + field_size);
            cursor += tuple_size;
        }
        end_checksum(attr[attr_idx].checksum, &column_checksum);
    }
}

void calc_column_checksum_dsm(
    timg_schema_t      schema,
    const void        *tuple_data,
    size_t             tuple_num)
{
    size_t             num_attr   = schema->attributes->num_elements;
    timg_attr_t       *attr       = (timg_attr_t *) schema->attributes->data;
    checksum_context_t column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t field_size = get_field_size(attr, attr_idx);
        size_t chunk_size = tuple_num * field_size;
        begin_checksum(&column_checksum);
        update_checksum(&column_checksum, tuple_data, tuple_data + chunk_size);
        end_checksum(attr[attr_idx].checksum, &column_checksum);
        tuple_data += chunk_size;
    }
}

void begin_checksum(checksum_context_t *context)
{
    MD5_Init (context);
}

void update_checksum(checksum_context_t *context, const void *begin, const void *end) {
    MD5_Update (context, begin, (end - begin));
}

void end_checksum(unsigned char *checksum_out, checksum_context_t *context)
{
    MD5_Final (checksum_out,context);
}


timg_error_t tableimg_header(
        FILE *file,
        timg_header_t *header)
{
    if (file == NULL || header == NULL)
        return TIMG_ERR_ILLEGALARG;

    int file_desc = fileno(file);
    fcntl(file_desc, F_GETFL);
    if (fcntl(file_desc, F_GETFL) == -1 && errno == EBADF)
        return TIMG_ERR_BADFILE;

    fseek(file, 0L, SEEK_END);
    if (ftell(file) < sizeof(timg_header_t))
        return TIMG_ERR_CORRUPTED;
    fseek(file, 0L, SEEK_SET);


    fread(header, sizeof(timg_header_t), 1, file);

    return TIMG_ERR_OK;
}