//
// Created by Mahmoud Mohsen on 24.01.18.
//

// here we vary the number of tuplets and the num
// of fields.

#include <stdio.h>

#include "exercice_helper.h"
#include "reference_implementation.h"

int main() {
    FILE *result_file = fopen("results_nsm_dsm_varying_tuplets_fields.csv", "a+");

    if (result_file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    apr_initialize();

    char *formats_str[2] ={"DSM", "NSM"};
    enum gs_frag_impl_type_e formats[] = {FIT_HOST_DSM_THIN_VM, FIT_HOST_NSM_FAT_VM};
    size_t num_fields = 1000;
    size_t fields_conf[] = {1, 100, 1000};
    size_t num_tuplets = 10000;

    for (int f = 0; f < 2; ++f) {

        for (size_t n_tuplets = 1; n_tuplets <= num_tuplets; n_tuplets+=1000) {
            printf("start: create the fragment \n");
            gs_frag_t *lineitem = help_create_lineitem_table(num_tuplets, num_fields, formats[f]);
            printf("start filling the fragment \n");
            help_fill_lineitem_table(lineitem, num_tuplets);
            printf("finish filling the fragment \n");

            for (size_t field_conf_idx = 0; field_conf_idx < 3; field_conf_idx++) {

                enum gs_boolean_operator_e *and_ors = GS_REQUIRE_MALLOC(sizeof( enum gs_boolean_operator_e)
                                                                         * fields_conf[field_conf_idx] - 1);
                enum gs_comp_type_e *cmp_types = GS_REQUIRE_MALLOC(sizeof( enum gs_comp_type_e) *
                                                                            fields_conf[field_conf_idx]);
                gs_attr_id_t *attr_ids = GS_REQUIRE_MALLOC(sizeof( gs_attr_id_t) * fields_conf[field_conf_idx]);
                uint32_t *cmp_vals = GS_REQUIRE_MALLOC(sizeof(uint32_t) * fields_conf[field_conf_idx]);

                *(cmp_types)  = CT_GREATEREQ;
                *(attr_ids)  = 0;
                *(cmp_vals)  = 0;

                for (gs_attr_id_t i = 0; i < fields_conf[field_conf_idx] - 1; ++i) {
                    *(and_ors + i) = BO_OR;
                    *(cmp_types + i + 1)  = CT_GREATEREQ;
                    *(attr_ids + i + 1)  = i + 1;
                    *(cmp_vals + i + 1)  = 0;
                }
                uint64_t timer_start = TIMER_STOP();
                gs_frag_raw_scan(lineitem, fields_conf[field_conf_idx] - 1, and_ors, cmp_types, attr_ids, cmp_vals);
                uint64_t timer_stop = TIMER_STOP();
                size_t time = timer_stop - timer_start;
                fprintf(result_file,"%zu,%zu,%zu,%s \n", n_tuplets,fields_conf[field_conf_idx], time, formats_str[f]);
                printf("%zu,%zu,%zu,%s \n", n_tuplets,fields_conf[field_conf_idx], time, formats_str[f]);
                free(and_ors);
                free(cmp_types);
                free(attr_ids);
                free(cmp_vals);
            }
            gs_frag_delete(lineitem);
        }
    }

    fclose(result_file);
    apr_terminate();
    return 0;
}