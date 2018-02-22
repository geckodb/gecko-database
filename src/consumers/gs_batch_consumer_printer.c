//
// Created by Mahmoud Mohsen on 27.12.17.
//

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <consumers/gs_batch_consumer_inter.h>
#include <gs_unsafe.h>
#include <gs_frag.h>
#include <gs_batch_consumer.h>
#include <gs_tuplet_field.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------
static gs_batch_consumer_t *create_consumer();
static void gs_batch_consumer_printer_print(gs_batch_consumer_t *self, void *batch, gs_frag_t *frag, size_t row_offset,
                                            size_t limit);
static void consumer_print_frag_header(FILE *file, const gs_frag_t *frag, gs_vec_t *field_print_lens, size_t num_attr);
static void consumer_print_frag_body_nsm_ids(FILE *file, void *batch, gs_frag_t *frag, gs_vec_t *field_print_lens,
                                     size_t num_attr,size_t limit);
static void consumer_calc_field_print_lens(gs_vec_t *field_print_lens, gs_frag_t *frag, size_t num_attr);
static void consumer_print_frag_body_dsm(FILE *file, void *ids_match, gs_frag_t *frag, gs_vec_t *field_print_lens,
                                  size_t num_attr, size_t num_matched_ids);
static void consumer_dispose(gs_batch_consumer_t *self);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

struct gs_batch_consumer_t *gs_batch_consumer_printer_new()
{
    return create_consumer();
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_batch_consumer_t *create_consumer()
{
    gs_batch_consumer_t *consumer = GS_REQUIRE_MALLOC(sizeof(gs_batch_consumer_t));
    *consumer = (gs_batch_consumer_t) {
        ._consume = gs_batch_consumer_printer_print,
        ._dispose = consumer_dispose
    };
    return consumer;
}

void consumer_dispose(gs_batch_consumer_t *self) {
    assert(self);
    free(self);
}

// the function to be assigned to the corresponding consumer -> consume ;D
void gs_batch_consumer_printer_print(gs_batch_consumer_t *self, void *batch, gs_frag_t *frag, size_t row_offset,
                                     size_t limit)
{
    assert(self);
    assert(batch);
    assert(frag);
    assert((row_offset >= 0));
    assert((limit >= 0));

    size_t num_attr = gs_frag_num_of_attributes(frag);
    gs_vec_t *field_print_lens = gs_vec_new(sizeof(size_t), num_attr + 1);
    gs_vec_resize(field_print_lens, num_attr);
    size_t zero = 0;
    gs_vec_memset(field_print_lens, 0, num_attr, &zero);
    consumer_calc_field_print_lens(field_print_lens, frag, num_attr);
    FILE *file = stdout;
    if (self->new_fragment) {
        consumer_print_frag_header(file, frag, field_print_lens, num_attr);
        self->new_fragment = false;
    }

    if (frag->impl_type == FIT_HOST_DSM_THIN_VM) {
        consumer_print_frag_body_dsm(file, batch, frag, field_print_lens, num_attr, limit);
    } else if (frag->impl_type == FIT_HOST_NSM_FAT_VM) {
        consumer_print_frag_body_nsm_ids(file, batch, frag, field_print_lens, num_attr, limit);
    } else {
        panic(NOTIMPLEMENTED,"no support for printing this types of fragments yet");
    }
    gs_vec_free(field_print_lens);
}


void consumer_calc_field_print_lens(gs_vec_t *field_print_lens, gs_frag_t *frag, size_t num_attr)
{

    size_t num_tuplets = frag->ntuplets;
    gs_schema_t *schema = gs_frag_schema(frag);

    while (num_tuplets--) {

        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
          //  enum gs_field_type_e type = gs_schema_attr_type(schema, attr_idx);
            const struct gs_attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
            size_t this_print_len_attr  = strlen(gs_attr_name(attr));
            size_t all_print_len = *(size_t *) gs_vec_at(field_print_lens, attr_idx);
            all_print_len = max(all_print_len, this_print_len_attr * 2);
            gs_vec_set(field_print_lens, attr_idx, 1, &all_print_len);
        }
    }
}


void consumer_print_h_line(FILE *file, const gs_frag_t *frag, size_t num_attr, gs_schema_t *schema, gs_vec_t *field_print_lens)
{
    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t   col_width = *(size_t *) gs_vec_at(field_print_lens, attr_idx);

        printf("+");
        for (size_t i = 0; i < col_width + 2; i++)
            printf("-");
    }

    printf("+\n");
}

void consumer_print_frag_header(FILE *file, const gs_frag_t *frag, gs_vec_t *field_print_lens, size_t num_attr)
{
    char format_buffer[2048];
    gs_schema_t *schema   = gs_frag_schema(frag);

    consumer_print_h_line(file, frag, num_attr, schema, field_print_lens);

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        const struct gs_attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
        size_t  col_width = *(size_t *) gs_vec_at(field_print_lens, attr_idx);
        sprintf(format_buffer, "| %%-%zus ", col_width);
        printf(format_buffer, gs_attr_name(attr));
    }
    printf("|\n");

    consumer_print_h_line(file, frag, num_attr, schema, field_print_lens);
}

void consumer_print_frag_body_nsm_ids(FILE *file, void *batch, gs_frag_t *frag, gs_vec_t *field_print_lens,
                                      size_t num_attr, size_t limit)
{

    char format_buffer[2048];
    gs_schema_t *schema = gs_frag_schema(frag);

    for (size_t k = 0; k < limit; ++k) {
        gs_attr_id_t matched_id = *(size_t *) (batch + (k * sizeof(gs_attr_id_t)) );
        gs_tuplet_t tuplet;
        gs_tuplet_open(&tuplet, frag, matched_id);
        struct gs_tuplet_field_t field;
        gs_tuplet_field_open(&field, &tuplet);
        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
            const gs_attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
            char *str = gs_unsafe_field_str(attr->type, gs_tuplet_field_read(&field));
            size_t print_len = max(strlen(str), *(size_t *) gs_vec_at(field_print_lens, attr_idx));
            sprintf(format_buffer, "| %%-%zus ", print_len);
            printf(format_buffer, str);
            free(str);
            gs_tuplet_field_next(&field, false);
        }

        printf("|\n");
    }

    consumer_print_h_line(file, frag, num_attr, schema, field_print_lens);

}


void consumer_print_frag_body_dsm(FILE *file, void *ids_match, gs_frag_t *frag, gs_vec_t *field_print_lens, size_t num_attr, size_t num_matched_ids)
{

    char format_buffer[2048];
//    size_t num_tuples = frag->ntuplets;
    gs_schema_t *schema = gs_frag_schema(frag);

    for (size_t k = 0; k < num_matched_ids; ++k) {
                size_t matched_id = *(size_t *) (ids_match + (k * sizeof(size_t)) );
                for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
                    const gs_attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
                    size_t attr_total_size = gs_attr_total_size(attr);
                    gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) frag->extras))->attr_vals_map,
                                                      attr, attr_total_size);

                    char *str = gs_unsafe_field_str(attr->type, gs_vec_at(attr_vals, matched_id));
                    size_t print_len = max(strlen(str), *(size_t *) gs_vec_at(field_print_lens, attr_idx));
                    sprintf(format_buffer, "| %%-%zus ", print_len);
                    printf(format_buffer, str);
                    free(str);
                }

                printf("|\n");
            }

    consumer_print_h_line(file, frag, num_attr, schema, field_print_lens);
}