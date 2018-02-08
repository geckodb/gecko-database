//
// Created by Mahmoud Mohsen on 24.01.18.
//

#include <stdio.h>

#include "exercice_helper.h"
#include "reference_implementation.h"

int main() {
    FILE *result_file = fopen("results00.txt", "a+");

    if (result_file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    apr_initialize();

    char *formats_str[2] ={"DSM", "NSM"};
    enum gs_frag_impl_type_e formats[] = {FIT_HOST_DSM_THIN_VM, FIT_HOST_NSM_VM};
    size_t num_fields = 1000;
    size_t fields_conf[] = {1, 100, 1000};
    size_t num_tuplets = 10000;

    for (int f = 0; f < 1; ++f) {

        for (size_t n_tuplets = 7000; n_tuplets <= 7000; n_tuplets+=1000 ) {
            gs_frag_t *lineitem = help_create_lineitem_table(num_tuplets, num_fields, formats[f]);
            help_fill_lineitem_table(lineitem, num_tuplets);

            for (size_t n_fields = 0; n_fields < 3; n_fields++) {
                gs_frag_t *q3 = NULL;
                uint64_t timer_start = TIMER_STOP();
                q3 = reference_scan_impl2(lineitem, fields_conf[n_fields], exercice_pred_greatereq, n_tuplets,&(uint64_t) {-1});
                uint64_t timer_stop = TIMER_STOP();
                size_t time = timer_stop - timer_start;
                fprintf(result_file,"%zu,%zu,%zu,%s \n", n_tuplets,fields_conf[n_fields], time, formats_str[f]);
                printf("%zu,%zu,%zu,%s \n", n_tuplets,fields_conf[n_fields], time, formats_str[f]);

            }
            gs_frag_delete(lineitem);
        }
    }
//    gs_frag_delete(q3);

//    gs_frag_print(stdout, q3, 0, num_tuplets);
    fclose(result_file);
    apr_terminate();
    return 0;
}