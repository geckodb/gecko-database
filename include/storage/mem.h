#pragma once

typedef size_t page_id_t;

typedef struct {
    void *data;
    size_t size;
} free_space_info_t;

typedef struct {
    size_t num_read, num_write, time_created, time_last_mode;
    size_t object_size, max_num_objects;
} data_block_header_t;

typedef struct {
    uint8_t is_free : 1;
    uint8_t is_forward : 1;
} data_block_slot_header_t;

typedef struct {
    page_id_t page_id;
    size_t page_size;
    list_t free_space;
    bool is_locked;
    void *data;
} managed_page_t;

typedef struct {
    managed_page_t *(*get_page)(page_id_t page);
    bool (*delete_page)(page_id_t page);
    page_id_t (*create_page)();
} managed_space_t;

typedef struct {
    managed_space_t base;
    vector_t *pages;
    list_t free_page_ids;
} vm_managed_space_t;

typedef struct {
    managed_space_t *space;
    size_t page_id, block_id;
} managed_ptr_t;

typedef enum {
    space_type_vm
} managed_space_type;

managed_space_t *mspace_create(managed_space_type type);

managed_ptr_t mspace_alloc(size_t safe_size, size_t num_safe_objects);

void mspace_resize(managed_ptr_t ptr, size_t size);

size_t mspace_safesize(managed_ptr_t ptr);

void *mspace_access(managed_ptr_t ptr);

void mspace_lock(managed_ptr_t);

void mspace_unlock(managed_ptr_t);