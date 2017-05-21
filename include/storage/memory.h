#pragma once

#include <storage/schema.h>

typedef unsigned schema_id_t;
#define NULL_TUPLE_ID UINT_MAX

typedef struct {
    schema_id_t schema_id;
    schema_t *schema;
} slot_schema_t;

typedef struct {
    struct {
        unsigned short is_forward_tuple : 1;
        unsigned short is_free          : 1;
        unsigned short schema_id        : 14;
    } header;
} fixed_tuple_slot_t;

typedef struct {
    struct {
        vector_t *schemas;
        vector_t *free_list_schema_ids;
    } schema_register;
    vector_t *pages;
} page_file_t;

typedef struct {
    void *begin, *end;
} free_space_entry_t;

typedef struct {

} slot_handle_t;

typedef struct {
    page_file_t *file;
    vector_t *slot_free_list;
    unsigned num_tuples;
    void *data_begin, *data_end, *first_tuple, *var_data_end;
} page_t;

typedef struct {
    page_t *page;
    unsigned slot;
} paged_tuple_handle_t;


page_file_t *page_file_create();

bool page_file_free(page_file_t *file);

page_t *page_create(page_file_t *file, size_t page_size);

bool page_free(page_t *page);

paged_tuple_handle_t page_write_tuple(page_t *page, const schema_t *schema, const void *data);