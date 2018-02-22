//
// Created by Mahmoud Mohsen on 27.12.17.
//

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_frag.h>
#include <consumers/gs_batch_consumer_inter.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum gs_batch_consumer_imp_e {
    CONSUMER_PRINTER, RAW_CONSUMER
};

typedef struct gs_batch_consumer_t {
    bool new_fragment;
    void (*_consume)(struct gs_batch_consumer_t *self, void *batch, struct gs_frag_t *frag, size_t row_offset,
                     size_t limit);
    void (*_dispose)(struct gs_batch_consumer_t *self);
    enum gs_batch_consumer_imp_e impl_type;
} gs_batch_consumer_t;

static struct consumer_type_pool_t {
    enum gs_batch_consumer_imp_e binding;
    gs_batch_consumer_t *(*_create)();
} consumer_type_pool[] = {
        { CONSUMER_PRINTER, gs_batch_consumer_printer_new },
        { RAW_CONSUMER, gs_batch_consumer_raw_new}
};

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_batch_consumer_t *gs_batch_consumer_create(enum gs_batch_consumer_imp_e vec_consumer_impl_type);
void gs_batch_consumer_dispose(gs_batch_consumer_t *consumer);
void gs_batch_consumer_consume(gs_batch_consumer_t *consumer, void *batch, struct gs_frag_t *frag, size_t row_offset,
                               size_t limit);
