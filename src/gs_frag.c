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

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static gs_vec_t *set_interect(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements);
static gs_vec_t *set_union(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_checksum_nsm(gs_schema_t *tab, const void *tuplets, size_t ntuplets)
{
    panic(NOTIMPLEMENTED, "this")
    /*
    size_t num_attr = tab->attr->num_elements;
    gs_attr_t *attr = (gs_attr_t *) tab->attr->data;
    size_t tuple_size = get_tuple_size(tab);
    gs_checksum_context_t column_checksum;

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

void gs_checksum_dms(gs_schema_t *tab, const void *tuplets, size_t ntuplets)
{
    panic(NOTIMPLEMENTED, "this")
    /*size_t num_attr = tab->attr->num_elements;
    gs_attr_t *attr = (gs_attr_t *) tab->attr->data;
    gs_checksum_context_t column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t field_size = get_field_size(attr, attr_idx);
        size_t chunk_size = ntuplets * field_size;
        begin_checksum(&column_checksum);
        update_checksum(&column_checksum, tuplets, tuplets + chunk_size);
        end_checksum(attr[attr_idx].checksum, &column_checksum);
        tuplets += chunk_size;
    }*/
}

void gs_checksum_begin(gs_checksum_context_t *context)
{
    MD5_Init (context);
}

void gs_checksum_update(gs_checksum_context_t *context, const void *begin, const void *end) {
    MD5_Update (context, begin, (end - begin));
}

void gs_checksum_end(unsigned char *checksum_out, gs_checksum_context_t *context)
{
    MD5_Final (checksum_out,context);
}

size_t find_type_match(enum gs_frag_impl_type_e type)
{
    size_t len = ARRAY_LEN_OF(frag_type_pool);
    for (size_t i = 0; i < len; i++) {
        if (frag_type_pool[i].binding == type) {
            return i;
        }
    }
    panic(BADFRAGTYPE, type);
}

enum gs_field_type_e gs_frag_field_type(const gs_frag_t *frag, gs_attr_id_t id)
{
    assert (frag);
    return gs_schema_attr_type(frag->schema, id);
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_frag_t *gs_frag_new(gs_schema_t *schema, size_t tuplet_capacity, enum gs_frag_impl_type_e type)
{
    panic_if(type != FIT_HOST_DSM_FAT_VM && type != FIT_HOST_NSM_FAT_VM && type != FIT_HOST_DSM_THIN_VM,
             NOTIMPLEMENTED, "type");
    REQUIRE((tuplet_capacity > 0), "capacity of tuplets must be non zero");

    gs_frag_t *result = frag_type_pool[find_type_match(type)]._create(schema, tuplet_capacity);
    result->impl_type = type;

    panic_if((result->_dispose == NULL), NOTIMPLEMENTED, "gs_frag_t::dispose");
    panic_if((result->_scan == NULL), NOTIMPLEMENTED, "gs_frag_t::scan");
    panic_if((result->_open == NULL), NOTIMPLEMENTED, "gs_frag_t::open");
    panic_if((result->_insert == NULL), NOTIMPLEMENTED, "gs_frag_t::this_query");
    return result;
}

void gs_frag_insert(struct gs_tuplet_t *out, gs_frag_t *frag, size_t ntuplets)
{
    assert (frag);
    assert (ntuplets > 0);
    assert (frag->_insert);

    gs_tuplet_t tmp;
    frag->_insert(&tmp, frag, ntuplets);
    GS_REQUIRE_NONNULL(tmp.fragment)
 //   GS_REQUIRE_NONNULL(tmp.attr_base)

    if (out != NULL) {
        *out = tmp;
    }
}

// the idea of the efficient set inersection is taken from:
// https://www.geeksforgeeks.org/find-union-and-intersection-of-two-unsorted-arrays/
// the union and intersection shall take
// min(mLogm + nLogm, mLogn + nLogn)
// TODO: add a hint about the expected selectivity factors of the predicates

void gs_frag_raw_scan(const gs_frag_t *frag, size_t num_boolean_operators, enum gs_boolean_operator_e *boolean_operators,
                      enum gs_comp_type_e *comp_type, gs_attr_id_t *attr_ids, const void *comp_vals)
{
    assert(frag);
    assert(boolean_operators);
    assert(comp_type);
    assert(attr_ids);
    assert(comp_vals);
    assert((num_boolean_operators >= 0));
    gs_vec_t *match_ids = gs_vec_new(sizeof(gs_attr_id_t), frag->ntuplets);
    frag->_raw_scan(frag, match_ids, *comp_type, *attr_ids, comp_vals);
            // dumy print to see if it is working or not
    for (size_t i = 0; i < num_boolean_operators; ++i) {
        enum gs_boolean_operator_e current_operator = *(boolean_operators + i);
        gs_vec_t *current_match_ids = gs_vec_new(sizeof(gs_attr_id_t), frag->ntuplets);
        frag->_raw_scan(frag, current_match_ids, *comp_type, *(attr_ids + 1 + i), comp_vals);
        if (current_operator == BO_AND) {
                match_ids = set_interect(match_ids, current_match_ids, gs_cmp_uint32);
            } else if (current_operator == BO_OR) {
                match_ids = set_union(match_ids, current_match_ids, gs_cmp_uint32);
            } else {
                panic(NOTIMPLEMENTED, "unsupported boolean operation");
            }
                gs_vec_dispose(current_match_ids);
            }
    size_t match_ids_length = gs_vec_length(match_ids);

    for (size_t j = 0; j < match_ids_length; ++j) {
        gs_tuplet_id_t *atuplet_id = gs_vec_at(match_ids, j);
        printf("found match {%u}\n", *atuplet_id);
    }
            gs_vec_dispose(match_ids);
}

void gs_frag_print(FILE *file, gs_frag_t *frag, size_t row_offset, size_t limit)
{
    gs_frag_print_ex(file, FPTT_CONSOLE_PRINTER, frag, row_offset, limit);
}

void gs_frag_print_ex(FILE *file, enum gs_frag_printer_type_tag_e type, gs_frag_t *frag, size_t row_offset,
                      size_t limit)
{
    gs_frag_printer_print(file, type, frag, row_offset, limit);
}

void gs_frag_delete(gs_frag_t *frag)
{
    assert(frag);
    frag->_dispose(frag);
}

const char *gs_frag_str(enum gs_frag_impl_type_e type)
{
    switch (type) {
        case FIT_HOST_NSM_FAT_VM: return "host/vm nsm";
        case FIT_HOST_DSM_FAT_VM: return "host/vm dsm";
        default: panic("Unknown fragment implementation type '%d'", type);
    }
}

size_t gs_frag_num_of_attributes(const gs_frag_t *frag)
{
    assert (frag);
    return gs_schema_num_attributes(frag->schema);
}

size_t gs_frag_num_of_tuplets(const gs_frag_t *frag)
{
    assert (frag);
    return frag->ntuplets;
}

gs_schema_t *gs_frag_schema(const gs_frag_t *frag)
{
    assert(frag);
    return frag->schema;
}


gs_vec_t *set_interect(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements) {

    assert(set1);
    assert(set2);
    assert(comp_elements);
    assert((set1->sizeof_element == set2->sizeof_element));

    size_t set1_length = gs_vec_length(set1);
    size_t set2_length = gs_vec_length(set2);
    gs_vec_t *result;

    if (set1_length < set2_length) {
        if (! set1->is_sorted)
            gs_vec_sort(set1, comp_elements);
        result = gs_vec_new(set1->sizeof_element, set1_length);

        for (size_t set2_pos = 0; set2_pos < set2_length; ++set2_pos) {
            void *set2_element = gs_vec_at(set2, set2_pos);
            if (gs_vec_contains_sorted(set1, set2_element, comp_elements))
                gs_vec_pushback(result, 1, set2_element);
        }

    } else {
        if (! set2->is_sorted)
            gs_vec_sort(set2, comp_elements);
        result = gs_vec_new(set2->sizeof_element,set2_length);
        for (size_t set1_pos = 0; set1_pos < set1_length; ++set1_pos) {
            void *set1_element = gs_vec_at(set1, set1_pos);
            if (gs_vec_contains_sorted(set2, set1_element, comp_elements))
                gs_vec_pushback(result, 1, set1_element);
        }
    }

    return result;
}

gs_vec_t *set_union(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements) {

    assert(set1);
    assert(set2);
    assert(comp_elements);
    assert((set1->sizeof_element == set2->sizeof_element));

    size_t set1_length = gs_vec_length(set1);
    size_t set2_length = gs_vec_length(set2);
    gs_vec_t *result;

    if (set1_length < set2_length) {
        if (! set1->is_sorted)
            gs_vec_sort(set1, comp_elements);
        result = gs_vec_new(set1->sizeof_element, set1_length);
        result = gs_vec_cpy_deep(set1);
        for (size_t set2_pos = 0; set2_pos < set2_length; ++set2_pos) {
            void *set2_element = gs_vec_at(set2, set2_pos);
            if (! gs_vec_contains_sorted(set1, set2_element, comp_elements))
                gs_vec_pushback(result, 1, set2_element);
        }

    } else {
        if (! set2->is_sorted)
            gs_vec_sort(set2, comp_elements);
        result = gs_vec_cpy_deep(set2);
        for (size_t set1_pos = 0; set1_pos < set1_length; ++set1_pos) {
            void *set1_element = gs_vec_at(set1, set1_pos);
            if (! gs_vec_contains_sorted(set2, set1_element, comp_elements))
                gs_vec_pushback(result, 1, set1_element);
        }
    }

    return result;
}