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

typedef struct frag_t {
    schema_t *schema; /*!< schema of this fragment */
    void *tuplet_data; /*!< data inside this fragment; the record format (e.g., NSM/DSM) is implementation-specific */
    size_t ntuplets; /*!< number of tuplets stored in this fragment */
    size_t ncapacity; /*!< number of tuplets that can be stored in this fragment, before a resize-opp is required */
    size_t tuplet_size; /*!< size in byte of a single tuplet */
    enum tuplet_format format; /*!< the tuplet format that defined whether NSM or DSM is used*/

    /* operations */
    struct frag_t *(*_scan)(struct frag_t *self, const pred_tree_t *pred, size_t batch_size, size_t nthreads);
    void (*_dispose)(struct frag_t *self);

    /*!< factory function to create impl-specific tuplet */
    struct tuplet_t *(*_open)(struct frag_t *self);

    /*!< inserts a number of (uninitialized) ntuplets into this fragment and returns a tuplet pointer to the first
     * tuplets of these newly added tuplets. */
    struct tuplet_t *(*_insert)(struct frag_t *self, size_t ntuplets);
} frag_t;

typedef struct frag_printer_t
{

} frag_printer_t;

// ---------------------------------------------------------------------------------------------------------------------
// M A R C O S
// ---------------------------------------------------------------------------------------------------------------------

#define DECLARE_ATTRIBUTE_CREATE(type_name,internal_type)                                                              \
attr_id_t gs_attr_create_##type_name(const char *name, schema_t *schema);

#define DECLARE_ATTRIBUTE_ARRAY_CREATE(type_name,internal_type)                                                        \
attr_id_t gs_attr_create_##type_name(const char *name, size_t length, schema_t *schema);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

schema_t *gs_schema_create();

void gs_schema_free(schema_t *schema);

frag_t *gs_fragment_alloc(schema_t *schema, size_t tuplet_capacity, enum tuplet_format format);

struct tuplet_t *gs_fragment_insert(frag_t *frag, size_t ntuplets);

void gs_fragment_free(frag_t *frag);

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

void gs_checksum_nsm(schema_t *tab, const void *tuplets, size_t ntuplets);

void gs_checksum_dms(schema_t *tab, const void *tuplets, size_t ntuplets);

void gs_checksum_begin(checksum_context_t *context);

void gs_checksum_update(checksum_context_t *context, const void *begin, const void *end);

void gs_checksum_end(unsigned char *checksum_out, checksum_context_t *context);

// F I E L D   T Y P E   O P E R A T I O N S ---------------------------------------------------------------------------

size_t gs_sizeof(enum field_type type);

const char *gs_type_str(enum field_type t);

__END_DECLS