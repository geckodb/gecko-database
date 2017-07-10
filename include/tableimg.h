#pragma once

#include <stdinc.h>
#include <attr.h>
#include <fragment.h>

#define TIMG_MAGIC_WORD_MAX 100
#define TIMG_MAGIC_WORD     "mptimg"
#define TIMG_FORMAT_VERSION   1

#define TIMG_FORMAT_NSM  0
#define TIMG_FORMAT_DSM  1





typedef struct {
    const char *dest_file;
    enum tuplet_format format;
} serialization_config_t;



typedef struct {
    char             magic[TIMG_MAGIC_WORD_MAX];
    uint8_t          format_version;

    uint32_t         database_name_len;     // w/o null terminator
    uint32_t         table_name_len;        // w/o null terminator
    uint32_t         table_spec_ref_len;    // w/o null terminator
    uint32_t         comment_len;           // w/o null terminator
    uint32_t         num_attributes_len;

    struct {
        uint8_t      serial_format_type : 1;
        uint8_t      reserved           : 8;
    } flags;

    uint64_t         num_tuples;
    uint64_t         timestamp_created;
    unsigned char    raw_table_data_checksum[MD5_DIGEST_LENGTH];
} timg_header_t;

typedef struct
{
    char             *database_name;
    char             *table_name;
    char             *table_spec_ref;
    char             *comment;
    ATTR         *attributes;
} timg_var_header_t;

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




timg_error_t tableimg_header_load(
        FILE *file,
        timg_header_t *header,
        timg_var_header_t *var_header
);

timg_error_t tableimg_header_free(
        timg_var_header_t *var_header
);

timg_error_t tableimg_fwrite(
        FILE *file,
        timg_version_t version,
        const char *database_name,
        const char *table_name,
        const char *table_spec_ref,
        const char *comment,
        SCHEMA *frag,
        const void *tuple_data,
        size_t num_tuples,
        enum tuplet_format format_in,
        enum tuplet_format format_out);



size_t tableimg_rawsize(
        timg_header_t *header,
        timg_var_header_t *var_header
);

