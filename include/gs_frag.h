// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gecko-commons/gecko-commons.h>
#include <gecko-commons/containers/gs_vec.h>

#include <gs_pred.h>
#include <gs_schema.h>
#include <gs_tuplet.h>
#include <gs_frag_printer.h>
#include <frags/gs_frag_host_vm.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

enum gs_frag_printer_type_tag_e;
struct gs_tuplet_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum gs_frag_impl_type_e {
    FIT_HOST_NSM_VM,
    FIT_HOST_DSM_VM
} gs_frag_impl_type_e;

typedef struct gs_frag_t {
    gs_schema_t *schema; /*!< schema of this fragment */
    void *tuplet_data; /*!< data inside this fragment; the record format (e.g., NSM/DSM) is implementation-specific */
    size_t ntuplets; /*!< number of tuplets stored in this fragment */
    size_t ncapacity; /*!< number of tuplets that can be stored in this fragment, before a resize-opp is required */
    size_t tuplet_size; /*!< size in byte of a single tuplet */
    enum gs_tuplet_format_e format; /*!< the tuplet format that defined whether NSM or DSM is used*/
    enum gs_frag_impl_type_e impl_type; /*!< the implementation type of the data fragment*/

    /* operations */
    struct gs_frag_t *(*_scan)(struct gs_frag_t *self, const gs_pred_tree_t *pred, size_t batch_size, size_t nthreads);
    void (*_dispose)(struct gs_frag_t *self);

    /*!< factory function to create impl-specific tuplet */
    void (*_open)(struct gs_tuplet_t *dst, struct gs_frag_t *self, gs_tuplet_id_t tuplet_id);

    /*!< inserts a number of (uninitialized) ntuplets into this fragment and returns a tuplet pointer to the first
     * tuplets of these newly added tuplets. */
    void (*_insert)(struct gs_tuplet_t *dst, struct gs_frag_t *self, size_t ntuplets);
} gs_frag_t;


static struct frag_type_pool_t {
    enum gs_frag_impl_type_e binding;
    gs_frag_t *(*_create)(gs_schema_t *schema, size_t tuplet_capacity);
} frag_type_pool[] = {
    { FIT_HOST_NSM_VM, gs_frag_host_vm_nsm_new },
    { FIT_HOST_DSM_VM, gs_frag_host_vm_dsm_new },
};

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

gs_frag_t *gs_frag_new(gs_schema_t *schema, size_t tuplet_capacity, enum gs_frag_impl_type_e type);
void gs_frag_delete(gs_frag_t *frag);

void gs_frag_insert(struct gs_tuplet_t *out, gs_frag_t *frag, size_t ntuplets);
void gs_frag_print(FILE *file, gs_frag_t *frag, size_t row_offset, size_t limit);
void gs_frag_print_ex(FILE *file, enum gs_frag_printer_type_tag_e printer_type, gs_frag_t *frag, size_t row_offset,
                      size_t limit);
const char *gs_frag_str(enum gs_frag_impl_type_e type);
size_t gs_frag_num_of_attributes(const gs_frag_t *frag);
size_t gs_frag_num_of_tuplets(const gs_frag_t *frag);
gs_schema_t *gs_frag_schema(const gs_frag_t *frag);
enum gs_field_type_e gs_frag_field_type(const gs_frag_t *frag, gs_attr_id_t id);

void gs_checksum_nsm(gs_schema_t *tab, const void *tuplets, size_t ntuplets);
void gs_checksum_dms(gs_schema_t *tab, const void *tuplets, size_t ntuplets);
void gs_checksum_begin(gs_checksum_context_t *context);
void gs_checksum_update(gs_checksum_context_t *context, const void *begin, const void *end);
void gs_checksum_end(unsigned char *checksum_out, gs_checksum_context_t *context);

// F I E L D   T Y P E   O P E R A T I O N S ---------------------------------------------------------------------------

size_t gs_field_type_sizeof(enum gs_field_type_e type);
const char *get_type_str(enum gs_field_type_e t);

__END_DECLS
