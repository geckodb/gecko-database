/*
 * Copyright (C) 2017 Marcus Pinnecke
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdinc.h>
#include <pred.h>
#include <schema.h>
#include <tuplet.h>
#include <containers/vector.h>

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct FRAGMENT {
    void *tuplet_data;
    size_t ntuplets;
    enum tuplet_format format;

    /* operations */
    struct FRAGMENT *(*_scan)(const PRED_TREE *pred, size_t batch_size, size_t nthreads);
    void (*_dispose)(struct FRAGMENT *self);
} FRAGMENT;

// ---------------------------------------------------------------------------------------------------------------------
// M A R C O S
// ---------------------------------------------------------------------------------------------------------------------

#define DECLARE_ATTRIBUTE_CREATE(type_name,internal_type)                                                              \
ATTR_ID gs_attr_create_##type_name(const char *name, SCHEMA *schema);

#define DECLARE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                        \
ATTR_ID gs_attr_create_##type_name(const char *name, size_t length, SCHEMA *schema);

#define DECLARE_TUPLET_INSERT(type_name, c_type, internal_type)                                                        \
void *gs_insert_##type_name(void *dst, SCHEMA *schema, ATTR_ID attr_id, const c_type *src);

#define DECLARE_ARRAY_FIELD_INSERT(type_name, c_type, internal_type)                                                   \
void *gs_insert_##type_name(void *dst, SCHEMA *schema, ATTR_ID attr_id, const c_type *src);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

// O P E R A T I O N S   O N   T A B L E S -----------------------------------------------------------------------------

SCHEMA *gs_schema_create();

FRAGMENT *gs_fragment_alloc(SCHEMA *frag, size_t num_tuplets, enum tuplet_format format);

void gs_fragment_free(FRAGMENT *frag);

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

void gs_checksum_nsm(SCHEMA *tab, const void *tuplets, size_t ntuplets);

void gs_checksum_dms(SCHEMA *tab, const void *tuplets, size_t ntuplets);


// O P E R A T I O N S   O N   R E C O R D S ---------------------------------------------------------------------------

DECLARE_TUPLET_INSERT(bool, bool, FT_BOOL)

DECLARE_TUPLET_INSERT(int8, int8_t, FT_INT8)

DECLARE_TUPLET_INSERT(int16, int16_t, FT_INT16)

DECLARE_TUPLET_INSERT(int32, int32_t, FT_INT32)

DECLARE_TUPLET_INSERT(int64, int64_t, FT_INT64)

DECLARE_TUPLET_INSERT(uint8, uint8_t, FT_UINT8)

DECLARE_TUPLET_INSERT(uint16, uint16_t, FT_UINT16)

DECLARE_TUPLET_INSERT(uint32, uint32_t, FT_UINT32)

DECLARE_TUPLET_INSERT(uint64, uint64_t, FT_UINT64)

DECLARE_TUPLET_INSERT(float32, float, FT_FLOAT32)

DECLARE_TUPLET_INSERT(float64, double, FT_FLOAT64)

DECLARE_ARRAY_FIELD_INSERT(string, char, FT_CHAR)

size_t gs_tuplet_printlen(const ATTR *attr, const void *field_data);

// F I E L D   T Y P E   O P E R A T I O N S ---------------------------------------------------------------------------

size_t gs_sizeof(enum field_type type);

const char *gs_type_str(enum field_type t);

__END_DECLS






















void gs_checksum_nsm(
        SCHEMA *tab,
        const void *tuplets,
        size_t ntuplets);

void gs_checksum_dms(
        SCHEMA *tab,
        const void *tuplets,
        size_t ntuplets);

void begin_checksum(CHECKSUM_CONTEXT *context);

void update_checksum(CHECKSUM_CONTEXT *context, const void *begin, const void *end);

void end_checksum(unsigned char *checksum_out, CHECKSUM_CONTEXT *context);

size_t get_field_size(
        const ATTR *attr,
        size_t             attr_idx);

size_t get_tuple_size(
        SCHEMA *schema);