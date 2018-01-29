#pragma once

#include <gs_attr.h>
#include <gs_frag.h>
#include <gs_tuplet_field.h>

#include "exercice_helper.h"
#include "reference_scan_impl.h"

static inline size_t reference_implementation(size_t num_tuples, size_t limit)
{
    gs_frag_t *lineitem = help_create_lineitem_table(num_tuples, 2,FIT_HOST_NSM_VM);
    help_fill_lineitem_table(lineitem,num_tuples);

    printf("$ SELECT * FROM lineitem LIMIT 10;\n");
    gs_frag_print(stdout, lineitem, 0, 10);

    printf("\n$ SELECT * FROM lineitem WHERE shipdate >= 1017679567 AND quantity < 24 AND discount BETWEEN 0.5 AND 0.7 LIMIT 15;\n");
    uint64_t timer_start = TIMER_STOP();
    gs_frag_t *q1 = reference_scan_impl(lineitem, "quantity", exercice_pred_less, &(uint32_t) {24});
    gs_frag_t *q2 = reference_scan_impl(q1, "shipdate", exercice_pred_greatereq, &(uint64_t) {1017679567});
    gs_frag_t *q3 = reference_scan_impl(q2, "discount", exercice_pred_greatereq, &(float) {0.5f});
    gs_frag_t *q4 = reference_scan_impl(q3, "discount", exercice_pred_lesseq, &(float) {0.7f});
    uint64_t timer_stop = TIMER_STOP();
    gs_frag_print(stdout, q4, 0, 15);
    size_t time = timer_stop - timer_start;
    printf("%zu records in %zums\n\n", gs_frag_num_of_tuplets (q4), time);

    gs_frag_delete(q4);
    gs_frag_delete(q3);
    gs_frag_delete(q2);
    gs_frag_delete(q1);
    gs_frag_delete(lineitem);
    return time;
}

