#pragma once

#include <stdatomic.h>
#include <limits.h>
#include "schema.h"

#define free_slot UINT_MAX;

typedef struct {
    void *slotted_page_forward;
    unsigned slots;
} forward_tuple_id_t;

typedef struct {
    struct {
        uint32_t is_forward_tuple_id : 1;
        uint32_t field_in_byte       : 30;
    } header;
    void *payload;
} page_slot_t;

typedef enum {
    mode_fixed_size,
    mode_variable_size
} addressing_mode;

typedef struct {
    schema_t *schema;
    unsigned page_size;
    vector_t *slotted_pages;
    vector_t *free_page_ids;
    addressing_mode mode;
} slotted_page_file_t;

typedef struct {
    struct {
        unsigned page_id;
        unsigned num_contained_tuples;
        atomic_bool is_locked;
        unsigned tuple_size;
    } header;
    page_slot_t *slots_begin, *slots_end;
} slotted_page_t;

slotted_page_file_t *slotted_page_file_create(schema_t *schema, unsigned page_size, unsigned num_pages);

bool slotted_page_file_free(slotted_page_file_t *file);