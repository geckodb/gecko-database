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
#include <frag_printer.h>
#include <containers/vector.h>
#include <frags/frag_host_vm.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D I N G
// ---------------------------------------------------------------------------------------------------------------------

enum frag_printer_type_tag;

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum frag_impl_type_t {
    FIT_HOST_NSM_VM,
    FIT_HOST_DSM_VM
};

typedef struct frag_t {
    schema_t *schema; /*!< schema of this fragment */
    void *tuplet_data; /*!< data inside this fragment; the record format (e.g., NSM/DSM) is implementation-specific */
    size_t ntuplets; /*!< number of tuplets stored in this fragment */
    size_t ncapacity; /*!< number of tuplets that can be stored in this fragment, before a resize-opp is required */
    size_t tuplet_size; /*!< size in byte of a single tuplet */
    enum tuplet_format format; /*!< the tuplet format that defined whether NSM or DSM is used*/
    enum frag_impl_type_t impl_type; /*!< the implementation type of the data fragment*/

    /* operations */
    struct frag_t *(*_scan)(struct frag_t *self, const pred_tree_t *pred, size_t batch_size, size_t nthreads);
    void (*_dispose)(struct frag_t *self);

    /*!< factory function to create impl-specific tuplet */
    struct tuplet_t *(*_open)(struct frag_t *self, tuplet_id_t tuplet_id);

    /*!< inserts a number of (uninitialized) ntuplets into this fragment and returns a tuplet pointer to the first
     * tuplets of these newly added tuplets. */
    struct tuplet_t *(*_insert)(struct frag_t *self, size_t ntuplets);
} frag_t;


static struct frag_type_pool_t {
    enum frag_impl_type_t binding;
    frag_t *(*_create)(schema_t *schema, size_t tuplet_capacity);
} frag_type_pool[] = {
    { FIT_HOST_NSM_VM, gs_frag_host_vm_nsm_create },
    { FIT_HOST_DSM_VM, gs_frag_host_vm_dsm_create },
};

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

frag_t *gs_fragment_alloc(schema_t *schema, size_t tuplet_capacity, enum frag_impl_type_t type);

void gs_frag_insert(struct tuplet_t *out, frag_t *frag, size_t ntuplets);

void gs_frag_print(FILE *file, frag_t *frag, size_t row_offset, size_t limit);

void gs_frag_print_ex(FILE *file, enum frag_printer_type_tag printer_type, frag_t *frag, size_t row_offset, size_t limit);

void gs_frag_free(frag_t *frag);

const char *gs_frag_str(enum frag_impl_type_t type);

size_t gs_fragment_num_of_attributes(const frag_t *frag);

size_t gs_fragment_num_of_tuplets(const frag_t *frag);

schema_t *gs_frag_get_schema(const frag_t *frag);

enum field_type gs_fragment_get_field_type(const frag_t *frag, attr_id_t id);

void gs_checksum_nsm(schema_t *tab, const void *tuplets, size_t ntuplets);

void gs_checksum_dms(schema_t *tab, const void *tuplets, size_t ntuplets);

void gs_checksum_begin(checksum_context_t *context);

void gs_checksum_update(checksum_context_t *context, const void *begin, const void *end);

void gs_checksum_end(unsigned char *checksum_out, checksum_context_t *context);

// F I E L D   T Y P E   O P E R A T I O N S ---------------------------------------------------------------------------

size_t gs_field_type_sizeof(enum field_type type);

const char *gs_type_str(enum field_type t);

__END_DECLS
