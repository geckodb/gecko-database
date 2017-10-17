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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_frag.h>
#include <frags/gs_frag_host_vm.h>
#include <gs_frag_printer.h>
#include <gs_schema.h>

void gs_checksum_nsm(schema_t *tab, const void *tuplets, size_t ntuplets)
{
    panic(NOTIMPLEMENTED, "this")
    /*
    size_t num_attr = tab->attr->num_elements;
    attr_t *attr = (attr_t *) tab->attr->data;
    size_t tuple_size = get_tuple_size(tab);
    checksum_context_t column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        const void *cursor = tuplets;
        panic(NOTIMPLEMENTED, "this")
        size_t field_offs = get_tuple_size(tab);
        size_t field_size = get_field_size(attr, attr_idx);
        begin_checksum(&column_checksum);
        for (size_t tuple_idx = 0; tuple_idx < num_attr; tuple_idx++) {
            update_checksum(&column_checksum, cursor + field_offs, cursor + field_offs + field_size);
            cursor += tuple_size;
        }
        end_checksum(attr[attr_idx].checksum, &column_checksum);
    }*/
}

void gs_checksum_dms(schema_t *tab, const void *tuplets, size_t ntuplets)
{
    panic(NOTIMPLEMENTED, "this")
    /*size_t num_attr = tab->attr->num_elements;
    attr_t *attr = (attr_t *) tab->attr->data;
    checksum_context_t column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t field_size = get_field_size(attr, attr_idx);
        size_t chunk_size = ntuplets * field_size;
        begin_checksum(&column_checksum);
        update_checksum(&column_checksum, tuplets, tuplets + chunk_size);
        end_checksum(attr[attr_idx].checksum, &column_checksum);
        tuplets += chunk_size;
    }*/
}

void gs_checksum_begin(checksum_context_t *context)
{
    MD5_Init (context);
}

void gs_checksum_update(checksum_context_t *context, const void *begin, const void *end) {
    MD5_Update (context, begin, (end - begin));
}

void gs_checksum_end(unsigned char *checksum_out, checksum_context_t *context)
{
    MD5_Final (checksum_out,context);
}

size_t find_type_match(enum frag_impl_type_t type)
{
    size_t len = ARRAY_LEN_OF(frag_type_pool);
    for (size_t i = 0; i < len; i++) {
        if (frag_type_pool[i].binding == type) {
            return i;
        }
    }
    panic(BADFRAGTYPE, type);
}


frag_t *frag_new(schema_t *schema, size_t tuplet_capacity, enum frag_impl_type_t type)
{
    REQUIRE((tuplet_capacity > 0), "capacity of tuplets must be non zero");

    frag_t *result = frag_type_pool[find_type_match(type)]._create(schema, tuplet_capacity);
    result->impl_type = type;

    panic_if((result->_dispose == NULL), NOTIMPLEMENTED, "frag_t::dispose");
    panic_if((result->_scan == NULL), NOTIMPLEMENTED, "frag_t::scan");
    panic_if((result->_open == NULL), NOTIMPLEMENTED, "frag_t::open");
    panic_if((result->_insert == NULL), NOTIMPLEMENTED, "frag_t::this_query");
    return result;
}

void frag_insert(struct tuplet_t *out, frag_t *frag, size_t ntuplets)
{
    assert (frag);
    assert (ntuplets > 0);
    assert (frag->_insert);

    tuplet_t tmp;
    frag->_insert(&tmp, frag, ntuplets);
    GS_REQUIRE_NONNULL(tmp.fragment)
    GS_REQUIRE_NONNULL(tmp.attr_base)

    if (out != NULL) {
        *out = tmp;
    }
}

void frag_print(FILE *file, frag_t *frag, size_t row_offset, size_t limit)
{
    frag_print_ex(file, FPTT_CONSOLE_PRINTER, frag, row_offset, limit);
}

void frag_print_ex(FILE *file, enum frag_printer_type_tag type, frag_t *frag, size_t row_offset, size_t limit)
{
    frag_printer_print(file, type, frag, row_offset, limit);
}

void frag_delete(frag_t *frag)
{
    assert(frag);
    frag->_dispose(frag);
}

const char *frag_str(enum frag_impl_type_t type)
{
    switch (type) {
        case FIT_HOST_NSM_VM: return "host/vm nsm";
        case FIT_HOST_DSM_VM: return "host/vm dsm";
        default: panic("Unknown fragment implementation type '%d'", type);
    }
}

size_t frag_num_of_attributes(const frag_t *frag)
{
    assert (frag);
    return schema_num_attributes(frag->schema);
}

size_t frag_num_of_tuplets(const frag_t *frag)
{
    assert (frag);
    return frag->ntuplets;
}

schema_t *frag_schema(const frag_t *frag)
{
    assert(frag);
    return frag->schema;
}

enum field_type frag_field_type(const frag_t *frag, attr_id_t id)
{
    assert (frag);
    return schema_attr_type(frag->schema, id);
}
