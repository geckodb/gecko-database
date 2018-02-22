//
// Created by Mahmoud Mohsen on 27.12.17.
//

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_batch_consumer.h>
#include <gs_unsafe.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void consumer_print_frag_header(FILE *file, const gs_frag_t *frag, gs_vec_t *field_print_lens, size_t num_attr);
void consumer_print_frag_body(FILE *file, void *batch, gs_frag_t *frag, gs_vec_t *field_print_lens, size_t num_attr,
                              size_t row_offset, size_t limit);
void consumer_calc_field_print_lens(gs_vec_t *field_print_lens, gs_frag_t *frag, size_t num_attr);
void consumer_print_frag_body_dsm(FILE *file, void *ids_match, gs_frag_t *frag, gs_vec_t *field_print_lens,
                                  size_t num_attr, size_t num_matched_ids);
// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

size_t find_consumer_type_match(enum gs_batch_consumer_imp_e batch_consumer_impl_type)
{
    size_t len = ARRAY_LEN_OF(frag_type_pool);
    for (size_t i = 0; i < len; i++) {
        if (consumer_type_pool[i].binding == batch_consumer_impl_type) {
            return i;
        }
    }
    panic("UNKOWN CONSUMER TYPE %d", batch_consumer_impl_type);
}

gs_batch_consumer_t *gs_batch_consumer_create(enum gs_batch_consumer_imp_e batch_consumer_impl_type)
{
    panic_if(batch_consumer_impl_type != RAW_CONSUMER && batch_consumer_impl_type != CONSUMER_PRINTER,
             NOTIMPLEMENTED, "vec_consumer_impl_type");

    return consumer_type_pool[find_consumer_type_match(batch_consumer_impl_type)]._create();

}

void gs_batch_consumer_dispose(gs_batch_consumer_t *consumer)
{
    assert(consumer);
    consumer->_dispose(consumer);
}

void gs_batch_consumer_consume(gs_batch_consumer_t *consumer, void *batch, gs_frag_t *frag, size_t row_offset,
                             size_t limit)
{
    assert(consumer);
    assert(batch);
    assert(frag);
    assert((row_offset >= 0));
    assert((limit >= 0));
    consumer->_consume(consumer, batch, frag, row_offset, limit);
}