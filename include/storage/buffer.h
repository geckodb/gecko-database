#pragma once

typedef struct {
    size_t offset, size;
} free_space_info_t;

typedef struct {
    struct {
        uint8_t is_free    : 1;
        uint8_t is_forward : 1;
    } flags;
} var_slot_header;

typedef uint32_t schema_id_t;
typedef uint32_t slotted_page_id_t;

typedef struct {
    struct {
        uint8_t is_free    : 1;
        uint8_t is_forward : 1;
    } flags;
    uint32_t size, capacity;
} fix_slot_header;

typedef struct {
    schema_id_t schema_id;
} regular_tuple_header_t;

typedef struct {
    slotted_page_id_t forwarded_to_page;
} forward_tuple_header_t;

typedef enum {
    space_shared,
    space_distinct
} fragment_space_policy;

typedef enum {
    size_policy_sumup,
    size_policy_each
} page_size_limit_policy;

typedef struct {
    struct {
        uint32_t page_id;
        uint32_t num_tuples;
        fragment_space_policy space_policy;
        uint32_t page_size;
    } header;
    struct {
        struct {
            list_t *free_space;
            void *data;
            size_t data_size;
        } var_fragment;
        struct {
            list_t *free_space;
            void *data;
            size_t data_size;
        } fix_fragment;
    } payload;
} slotted_page_t;

slotted_page_t *slotted_page_create(uint32_t page_id, uint32_t page_size, fragment_space_policy policy);

bool slotted_page_free(slotted_page_t *page);