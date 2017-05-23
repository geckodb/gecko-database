#pragma once

typedef struct {

} page_file_t;

typedef struct {
    void *begin, *end;
} space_region_t;

typedef struct {
    list_t *free_space;
} page_header_t;

typedef struct {
    page_file_t *file;
    size_t page_id;
} page_ptr;

typedef struct {
    size_t per_zone_element_size;
    size_t per_zone_element_capacity;
} frame_header_t;

typedef struct {
    page_ptr context;
    size_t offset;
} frame_ptr;

typedef struct {
    frame_ptr context;
    size_t offset;
} zone_ptr;

typedef struct {
    zone_ptr prev, next;
} zone_header_t;
