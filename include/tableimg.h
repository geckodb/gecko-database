#pragma once

#include <types.h>
#include <openssl/md5.h>
#include <containers/vector.h>

#define TIMG_MAGIC_WORD_MAX 100
#define TIMG_MAGIC_WORD     "mptimg"
#define TIMG_FORMAT_VERSION   1

#define TIMG_FORMAT_NSM  0
#define TIMG_SERIALIZATION_FORMAT_DSM  1

#define TIMG_ATTRIBUTE_NAME_MAX         1024
#define TIMG_DATABASE_NAME_MAX          1024
#define TIMG_TABLE_NAME_MAX             1024
#define TIMG_TABLE_SPEC_REF_MAX         2048
#define TIMG_ATTRIBUTE_NUM_MAX          1024
#define TIMG_COMMENT_MAX                1024

#define DECLARE_ATTRIBUTE_CREATE(type_name,internal_type)                       \
timg_attr_id_t timg_attribute_create_##type_name(                               \
    const char       *name,                                                     \
    timg_schema_t     schema                                                    \
);

#define DECLARE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                 \
timg_attr_id_t timg_attribute_create_##type_name(                               \
    const char       *name,                                                     \
    size_t            length,                                                   \
    timg_schema_t     schema                                                    \
);

#define DECLARE_FIELD_WRITE(type_name, c_type, internal_type)                                               \
void *timg_field_write_##type_name(void *dst, const timg_schema_t schema, timg_attr_id_t attr_id, const c_type *src);

#define DECLARE_ARRAY_FIELD_WRITE(type_name, c_type, internal_type)                                               \
void *timg_field_write_##type_name(void *dst, const timg_schema_t schema, timg_attr_id_t attr_id, const c_type *src);

typedef enum {
    TIMG_DATATYPE_BOOL            =   0,
    TIMG_DATATYPE_INT8            =   1,
    TIMG_DATATYPE_INT16           =   2,
    TIMG_DATATYPE_INT32           =   3,
    TIMG_DATATYPE_INT64           =   4,
    TIMG_DATATYPE_UINT8           =   5,
    TIMG_DATATYPE_UINT16          =   6,
    TIMG_DATATYPE_UINT32          =   7,
    TIMG_DATATYPE_UINT64          =   8,
    TIMG_DATATYPE_FLOAT32         =   9,
    TIMG_DATATYPE_FLOAT64         =  10,
    TIMG_DATATYPE_STRING          =  11,
} timg_data_type_t;

typedef enum  {
    SF_NSM, SF_DSM
} timg_format_t;

typedef struct {
    const char *dest_file;
    timg_format_t format;
} serialization_config_t;

typedef struct {
    uint8_t       is_primary_key : 1;
    uint8_t       is_foreign_key : 1;
    uint8_t       is_nullable    : 1;
    uint8_t       is_autoinc     : 1;
    uint8_t       is_unique      : 1;
} attr_flags_t;

typedef struct {
    size_t            unique_id;
    char              name[TIMG_ATTRIBUTE_NAME_MAX];
    timg_data_type_t  data_type;
    size_t            data_type_rep;
    attr_flags_t      attr_flags;
    size_t            foreign_unique_id;
    unsigned char     checksum[MD5_DIGEST_LENGTH];
} timg_attr_t;

struct timg_schema {
    vector_t         *attributes;
};

typedef struct timg_schema *timg_schema_t;

typedef uint64_t timg_attr_id_t;



typedef struct {
    char             magic[TIMG_MAGIC_WORD_MAX];
    uint8_t          format_version;

    char             database_name[TIMG_DATABASE_NAME_MAX];
    char             table_name[TIMG_TABLE_NAME_MAX];
    char             table_spec_ref[TIMG_TABLE_SPEC_REF_MAX];
    char             comment[TIMG_COMMENT_MAX];

    timg_attr_t      attributes[TIMG_ATTRIBUTE_NUM_MAX];
    size_t           num_attributes;

    struct {
        uint8_t      serial_format_type : 1;
        uint8_t      reserved           : 8;
    } flags;

    uint64_t         num_tuples;
    uint64_t         timestamp_created;
    unsigned char    raw_table_data_checksum[MD5_DIGEST_LENGTH];
} timg_header_t;

typedef enum {
    TIMG_ERR_OK                 =    1,
    TIMG_ERR_ILLEGALARG         =    2,
    TIMG_ERR_DBNAME_TOOLONG     =    3,
    TIMG_ERR_TABNAME_TOOLONG    =    4,
    TIMG_ERR_TABSPEC_TOOLONG    =    5,
    TIMG_ERR_BADFILE            =    6,
    TIMG_ERR_TOMANYATTRIBUTES   =    7,
    TIMG_ERR_BADSERIALFORMAT    =    8,
    TIMG_ERR_CORRUPTED          =    9,
    TIMG_ERR_COMMENT_TOOLONG    =   10,

} timg_error_t;

typedef enum {
    TIMG_VER_1 = 1
} timg_version_t;


timg_schema_t timg_schema_create();

timg_error_t tableimg_header(
        FILE *file,
        timg_header_t *header);

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
        timg_format_t format_out);

void *timg_table_create(
        timg_schema_t schema,
        size_t num_tuples);

size_t tableimg_rawsize(
        timg_header_t *header
);

const char *tableimg_datatype_str(timg_data_type_t t);

DECLARE_ATTRIBUTE_CREATE(bool, TIMG_DATATYPE_BOOL)
DECLARE_ATTRIBUTE_CREATE(int8, TIMG_DATATYPE_INT8)
DECLARE_ATTRIBUTE_CREATE(int16, TIMG_DATATYPE_INT16)
DECLARE_ATTRIBUTE_CREATE(int32, TIMG_DATATYPE_INT32)
DECLARE_ATTRIBUTE_CREATE(int64, TIMG_DATATYPE_INT64)
DECLARE_ATTRIBUTE_CREATE(uint8, TIMG_DATATYPE_UINT8)
DECLARE_ATTRIBUTE_CREATE(uint16, TIMG_DATATYPE_UINT16)
DECLARE_ATTRIBUTE_CREATE(uint32, TIMG_DATATYPE_UINT32)
DECLARE_ATTRIBUTE_CREATE(uint64, TIMG_DATATYPE_UINT64)
DECLARE_ATTRIBUTE_CREATE(float32, TIMG_DATATYPE_FLOAT32)
DECLARE_ATTRIBUTE_CREATE(float64, TIMG_DATATYPE_FLOAT64)
DECLARE_ATTRIBUTE_ARRAY_CREATE(string, TIMG_DATATYPE_STRING)

DECLARE_FIELD_WRITE(bool, bool, TIMG_DATATYPE_BOOL)
DECLARE_FIELD_WRITE(int8, int8_t, TIMG_DATATYPE_INT8)
DECLARE_FIELD_WRITE(int16, int16_t, TIMG_DATATYPE_INT16)
DECLARE_FIELD_WRITE(int32, int32_t, TIMG_DATATYPE_INT32)
DECLARE_FIELD_WRITE(int64, int64_t, TIMG_DATATYPE_INT64)
DECLARE_FIELD_WRITE(uint8, uint8_t, TIMG_DATATYPE_UINT8)
DECLARE_FIELD_WRITE(uint16, uint16_t, TIMG_DATATYPE_UINT16)
DECLARE_FIELD_WRITE(uint32, uint32_t, TIMG_DATATYPE_UINT32)
DECLARE_FIELD_WRITE(uint64, uint64_t, TIMG_DATATYPE_UINT64)
DECLARE_FIELD_WRITE(float32, float, TIMG_DATATYPE_FLOAT32)
DECLARE_FIELD_WRITE(float64, double, TIMG_DATATYPE_FLOAT64)
DECLARE_ARRAY_FIELD_WRITE(string, char, TIMG_DATATYPE_STRING)