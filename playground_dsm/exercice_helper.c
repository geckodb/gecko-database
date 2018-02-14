
#include <gs_frag.h>
#include <gs_tuplet_field.h>
#include <gs_schema.h>

#include "exercice_helper.h"

gs_frag_t *help_create_lineitem_table(size_t capacity,size_t num_fields,enum gs_frag_impl_type_e type) {

    gs_schema_t *lineitem_schema = gs_schema_new("lineitem");
    char att_name[10000];

    for (size_t i = 0; i < num_fields; ++i) {
        sprintf(att_name, "%zu", i);
        attr_create_uint32(att_name, lineitem_schema);
    }

    gs_frag_t *result = gs_frag_new(lineitem_schema, capacity, type);
    gs_schema_delete(lineitem_schema);
    return result;
}

void help_fill_lineitem_table(gs_frag_t *frag, size_t num_tuples) {
    gs_tuplet_t tuplet;
    gs_tuplet_field_t field;

    for (unsigned i = 0; i < num_tuples; i++) {
        uint32_t col_0 = i;
        gs_frag_insert(&tuplet, frag, 1);
        gs_tuplet_field_open(&field, &tuplet);
        gs_tuplet_field_write(&field, &col_0, true);
        for (int j = 1; j < gs_schema_num_attributes(frag->schema); ++j) {
            uint32_t col_j = num_tuples - i;
            gs_tuplet_field_write(&field, &col_j, true);
        }
    }
}
