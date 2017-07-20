#pragma once

#include <stdinc.h>
#include <schema.h>
#include <field_type.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A R C O S
// ---------------------------------------------------------------------------------------------------------------------

#define DECLARE_ATTRIBUTE_CREATE(type_name,internal_type)                                                              \
attr_id_t gs_attr_create_##type_name(const char *name, schema_t *schema);

#define DECLARE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                        \
attr_id_t gs_attr_create_##type_name(const char *name, size_t length, schema_t *schema);

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct attr_t {
    size_t id;
    char name[ATTR_NAME_MAXLEN];
    enum field_type type;
    size_t type_rep;
    ATTR_FLAGS flags;
    size_t foreign_id;
    size_t str_format_mlen;
    unsigned char checksum[MD5_DIGEST_LENGTH];
} attr_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

const char *gs_attr_get_name(const struct attr_t *attr);

size_t gs_attr_get_str_format_max_len(attr_t *attr);

enum field_type gs_attr_get_type(const attr_t *attr);

attr_t *gs_attr_cpy(const attr_t *template, schema_t *new_owner);

DECLARE_ATTRIBUTE_CREATE(bool, FT_BOOL)

DECLARE_ATTRIBUTE_CREATE(int8, FT_INT8)

DECLARE_ATTRIBUTE_CREATE(int16, FT_INT16)

DECLARE_ATTRIBUTE_CREATE(int32, FT_INT32)

DECLARE_ATTRIBUTE_CREATE(int64, FT_INT64)

DECLARE_ATTRIBUTE_CREATE(uint8, FT_UINT8)

DECLARE_ATTRIBUTE_CREATE(uint16, FT_UINT16)

DECLARE_ATTRIBUTE_CREATE(uint32, FT_UINT32)

DECLARE_ATTRIBUTE_CREATE(uint64, FT_UINT64)

DECLARE_ATTRIBUTE_CREATE(float32, FT_FLOAT32)

DECLARE_ATTRIBUTE_CREATE(float64, FT_FLOAT64)

DECLARE_ATTRIBUTE_ARRAY_CREATE(string, FT_CHAR)


