//
// Created by Mahmoud Mohsen on 24.01.18.
//

#include <stdio.h>

#include "exercice_helper.h"
#include "reference_implementation.h"
#include <gecko-commons/gs_hash.h>

int main() {
    apr_initialize();
    size_t num_tuplets = 10;
    size_t num_fields = 2;
    gs_frag_t *lineitem = help_create_lineitem_table(num_tuplets, num_fields, FIT_HOST_DSM_THIN_VM);
    help_fill_lineitem_table(lineitem, num_tuplets);
    gs_frag_print(stdout, lineitem, 0, num_tuplets);
    gs_frag_delete(lineitem);
    apr_terminate();
    return 0;
}