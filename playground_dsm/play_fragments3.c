//
// Created by Mahmoud Mohsen on 24.01.18.
//

#include <stdio.h>

#include "exercice_helper.h"
#include "reference_implementation.h"
#include <gecko-commons/gs_hash.h>

//static gs_vec_t *set_interect(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements);
//static gs_vec_t *set_union(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements);

//static int compare_uint32 (const void *lhs, const void *rhs);

int main() {
    apr_initialize();

    size_t num_tuplets = 10;
    size_t num_fields = 2;
    gs_frag_t *lineitem = help_create_lineitem_table(num_tuplets, num_fields, FIT_HOST_NSM_FAT_VM);
    help_fill_lineitem_table(lineitem, num_tuplets);
    enum gs_boolean_operator_e *and_ors  = GS_REQUIRE_MALLOC(sizeof( enum gs_boolean_operator_e) * 1);
    enum gs_comp_type_e *cmp_types  = GS_REQUIRE_MALLOC(sizeof( enum gs_comp_type_e) * 2);
    gs_attr_id_t *attr_ids  = GS_REQUIRE_MALLOC(sizeof( gs_attr_id_t) * 2);
    uint32_t *cmp_vals  = GS_REQUIRE_MALLOC(sizeof(uint32_t) * 2);
    *and_ors = BO_OR;
    *cmp_types = CT_GREATEREQ;
    *(cmp_types + 1) = CT_GREATEREQ;
    *attr_ids = 0;
    *(attr_ids + 1) = 1;
    *cmp_vals = 5;
    *(cmp_vals + 1) = 2;
    gs_frag_print(stdout, lineitem, 0, 15);

    gs_batch_consumer_t *console_printer_consumer = gs_batch_consumer_create(CONSUMER_PRINTER);
 //   gs_frag_raw_scan(lineitem, 1, and_ors, cmp_types, attr_ids, cmp_vals);
    gs_frag_raw_vectorized_scan(lineitem, console_printer_consumer, 1, and_ors, cmp_types, attr_ids, cmp_vals, 10,
                               false);
    free(and_ors);
    free(cmp_types);
    free(attr_ids);
    free(cmp_vals);
    gs_frag_delete(lineitem);
    gs_batch_consumer_dispose(console_printer_consumer);
//    gs_vec_t *set1;
//    gs_vec_t *set2;
//    gs_vec_t *set1_2_intersection;
//    gs_vec_t *set1_2_union;
//
//    set1 = gs_vec_new(sizeof( uint32_t), 4);
//    set2 = gs_vec_new(sizeof( uint32_t), 3);
//
//    gs_vec_pushback(set1, 1, &(uint32_t) {0});
//    gs_vec_pushback(set1, 1, &(uint32_t) {1});
//    gs_vec_pushback(set1, 1, &(uint32_t) {2});
//    gs_vec_pushback(set1, 1, &(uint32_t) {3});
//
//    gs_vec_pushback(set2, 1, &(uint32_t) {5});
//    gs_vec_pushback(set2, 1, &(uint32_t) {7});
//    gs_vec_pushback(set2, 1, &(uint32_t) {9});
//
//    set1_2_intersection = set_interect(set1, set2, compare_uint32);
//    set1_2_union = set_union(set1, set2, compare_uint32);
//    gs_vec_dispose(set1_2_intersection);
//    gs_vec_dispose(set1_2_union);
//    gs_vec_dispose(set1);
//    gs_vec_dispose(set2);

    apr_terminate();
    return 0;
}

//int compare_uint32 (const void *lhs, const void *rhs) {
//    if (*(uint32_t *)lhs > *(uint32_t *)rhs) return 1;
//    if (*(uint32_t *)lhs < *(uint32_t *)rhs) return -1;
//    return 0;
//}

//gs_vec_t *set_interect(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements) {
//
//    assert(set1);
//    assert(set2);
//    assert(comp_elements);
//    assert((set1->sizeof_element == set2->sizeof_element));
//
//    size_t set1_length = gs_vec_length(set1);
//    size_t set2_length = gs_vec_length(set2);
//    gs_vec_t *result;
//
//    if (set1_length < set2_length) {
//        if (! set1->is_sorted)
//        gs_vec_sort(set1, comp_elements);
//        result = gs_vec_new(set1->sizeof_element, set1_length);
//
//        for (size_t set2_pos = 0; set2_pos < set2_length; ++set2_pos) {
//            void *set2_element = gs_vec_at(set2, set2_pos);
//            if (gs_vec_contains_sorted(set1, set2_element, comp_elements))
//                gs_vec_pushback(result, 1, set2_element);
//        }
//
//    } else {
//        if (! set2->is_sorted)
//            gs_vec_sort(set2, comp_elements);
//        result = gs_vec_new(set2->sizeof_element,set2_length);
//        for (size_t set1_pos = 0; set1_pos < set1_length; ++set1_pos) {
//            void *set1_element = gs_vec_at(set1, set1_pos);
//            if (gs_vec_contains_sorted(set2, set1_element, comp_elements))
//                gs_vec_pushback(result, 1, set1_element);
//        }
//    }
//
//    return result;
//}
//
//gs_vec_t *set_union(gs_vec_t *set1, gs_vec_t *set2, gs_comp_t comp_elements) {
//
//    assert(set1);
//    assert(set2);
//    assert(comp_elements);
//    assert((set1->sizeof_element == set2->sizeof_element));
//
//    size_t set1_length = gs_vec_length(set1);
//    size_t set2_length = gs_vec_length(set2);
//    gs_vec_t *result;
//
//    if (set1_length < set2_length) {
//        if (! set1->is_sorted)
//            gs_vec_sort(set1, comp_elements);
//        result = gs_vec_new(set1->sizeof_element, set1_length);
//        result = gs_vec_cpy_deep(set1);
//        for (size_t set2_pos = 0; set2_pos < set2_length; ++set2_pos) {
//            void *set2_element = gs_vec_at(set2, set2_pos);
//            if (! gs_vec_contains_sorted(set1, set2_element, comp_elements))
//                gs_vec_pushback(result, 1, set2_element);
//        }
//
//    } else {
//        if (! set2->is_sorted)
//            gs_vec_sort(set2, comp_elements);
//        result = gs_vec_cpy_deep(set2);
//        for (size_t set1_pos = 0; set1_pos < set1_length; ++set1_pos) {
//            void *set1_element = gs_vec_at(set1, set1_pos);
//            if (! gs_vec_contains_sorted(set2, set1_element, comp_elements))
//                gs_vec_pushback(result, 1, set1_element);
//        }
//    }
//
//    return result;
//}