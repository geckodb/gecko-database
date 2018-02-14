#pragma once

#include <gs_attr.h>
#include <gs_frag.h>
#include <gs_tuplet_field.h>
#include "exercice_helper.h"

static inline gs_frag_t *reference_scan_impl(gs_frag_t *table, const char *attr_name, exercice_pred_e pred,
                                               const void *data) {
    gs_tuplet_t input_tuplet, output_tuplet;
    gs_tuplet_field_t input_field, output_field;
    gs_tuplet_open(&input_tuplet, table, 0);

    const gs_attr_t *attribute = gs_schema_attr_by_name(table->schema, attr_name);
    exercice_cmp_func_t cmp_func;

    switch (attribute->type) {
        case FT_INT32:
            cmp_func = exercice_cmp_int32;
            break;
        case FT_UINT32:
            cmp_func = exercice_cmp_uint32;
            break;
        case FT_UINT64:
            cmp_func = exercice_cmp_uint64;
            break;
        case FT_FLOAT32:
            cmp_func = exercice_cmp_float32;
            break;
        case FT_CHAR:
            cmp_func = exercice_cmp_char;
            break;
        default: panic("not implemented '%d'", attribute->type);
    }
    // need to change for the DSM
    gs_frag_t *result = gs_frag_new(table->schema, table->ntuplets, FIT_HOST_NSM_FAT_VM);

    do {
        gs_tuplet_field_open(&input_field, &input_tuplet);
        gs_tuplet_field_seek(&input_field, &input_tuplet, attribute->id);
        const void *field_data = gs_tuplet_field_read(&input_field);
        int compare = cmp_func(field_data, data);
        bool match = ((pred == exercice_pred_less && compare < 0) ||
                      (pred == exercice_pred_greater && compare > 0) ||
                      (pred == exercice_pred_equals && compare == 0) ||
                      (pred == exercice_pred_lesseq && compare <= 0) ||
                      (pred == exercice_pred_greatereq && compare >= 0) ||
                      (pred == exercice_pred_unequal && compare != 0));
        if (match) {
            gs_tuplet_field_seek(&input_field, &input_tuplet, 0);
            gs_frag_insert(&output_tuplet, result, 1);
            gs_tuplet_field_open(&output_field, &output_tuplet);

            for (int i = 0; i < gs_schema_num_attributes(table->schema); i++) {
                const void *in_data = gs_tuplet_field_read(&input_field);
                gs_tuplet_field_write(&output_field,
                                      gs_schema_attr_type(table->schema, i) != FT_CHAR ? in_data : &in_data, true);
                gs_tuplet_field_next(&input_field, false);
            }
        }

    } while (gs_tuplet_next(&input_tuplet));
    return result;
}



static inline gs_frag_t *reference_scan_impl2(gs_frag_t *table, size_t num_attrs, exercice_pred_e pred,
                                             size_t num_tuplets,const void *data) {
    gs_tuplet_t input_tuplet, output_tuplet;
    gs_tuplet_field_t input_field, output_field;
    // need to change for the DSM
    gs_frag_t *result = gs_frag_new(table->schema, table->ntuplets, table->impl_type);

    do {
        gs_tuplet_open(&input_tuplet, table, random()% num_tuplets);
        bool match = true;
        gs_tuplet_field_open(&input_field, &input_tuplet);
        for (int j = 0; j < num_attrs; ++j) {
            const gs_attr_t *attribute = gs_schema_attr_by_id(table->schema, j);
            exercice_cmp_func_t cmp_func;

            switch (attribute->type) {
                case FT_INT32:
                    cmp_func = exercice_cmp_int32;
                    break;
                case FT_UINT32:
                    cmp_func = exercice_cmp_uint32;
                    break;
                case FT_UINT64:
                    cmp_func = exercice_cmp_uint64;
                    break;
                case FT_FLOAT32:
                    cmp_func = exercice_cmp_float32;
                    break;
                case FT_CHAR:
                    cmp_func = exercice_cmp_char;
                    break;
                default: panic("not implemented '%d'", attribute->type);
            }

            gs_tuplet_field_seek(&input_field, &input_tuplet, attribute->id);
            const void *field_data = gs_tuplet_field_read(&input_field);
            int compare = cmp_func(field_data, data);
            match &= ((pred == exercice_pred_less && compare < 0) ||
                     (pred == exercice_pred_greater && compare > 0) ||
                     (pred == exercice_pred_equals && compare == 0) ||
                     (pred == exercice_pred_lesseq && compare <= 0) ||
                     (pred == exercice_pred_greatereq && compare >= 0) ||
                     (pred == exercice_pred_unequal && compare != 0));

        }
        if (match) {
            gs_tuplet_field_seek(&input_field, &input_tuplet, 0);
            gs_frag_insert(&output_tuplet, result, 1);
            gs_tuplet_field_open(&output_field, &output_tuplet);

            for (int i = 0; i < gs_schema_num_attributes(table->schema); i++) {
                const void *in_data = gs_tuplet_field_read(&input_field);
                gs_tuplet_field_write(&output_field,
                                      gs_schema_attr_type(table->schema, i) != FT_CHAR ? in_data : &in_data, true);
                gs_tuplet_field_next(&input_field, false);
            }
        }

    } while (--num_tuplets);
    return result;
}
