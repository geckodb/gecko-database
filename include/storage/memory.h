// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <defs.h>
#include <containers/vector.h>
#include <containers/dictionaries/fixed_linear_hash_table.h>

#define __force_packing__        __attribute__((__packed__))

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define PAGEPOOL_INITCAP        100
#define PAGEPOOL_GROW_FACTOR  2.00f
#define PAGEPOOL_MAX_FILL_FAC 0.75f

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef u32 page_id_t;
typedef u32 frame_id_t;
typedef u32 zone_id_t;

typedef size_t offset_t;

typedef struct {
    offset_t begin, end;
} range_t;

typedef struct __force_packing__ {
    struct __force_packing__ {
        page_id_t is_far_ptr    : 1;
        page_id_t page_id       : 31;
    };
    struct __force_packing__ {
        offset_t target_type_bit_0    : 1;
        offset_t target_type_bit_1    : 1;
        offset_t offset               : 62;
    };
} in_page_ptr;

typedef enum
{
    target_frame,
    target_zone,
    target_userdata,
    target_corrupted,
} ptr_target;

typedef enum {
    page_flag_dirty = 1 << 1,
    page_flag_fixed = 1 << 2,
    page_flag_locked = 1 << 3
} page_flags;

typedef struct __force_packing__ {
    u64 time_created, time_last_read, time_last_write;
    u64 num_reads, num_writes;
} access_stats_t;

typedef struct __force_packing__ {
    page_id_t page_id;
    size_t page_size, free_space;
    access_stats_t statistics;
    struct {
        u8 is_dirty  : 1;
        u8 is_fixed  : 1;
        u8 is_locked : 1;
    } flags;
} page_header_t;

typedef struct __force_packing__ {
    u32 list_max, list_len;
} free_store_t;

typedef struct __force_packing__ {
    u32 max_num_frames, in_use_num, free_list_len;
} frame_store_t;

typedef struct __force_packing__ {
    in_page_ptr first, last;
    size_t elem_size;
} frame_t;

typedef struct __force_packing__ {
    in_page_ptr prev, next;
    access_stats_t statistics;
} zone_t;

typedef struct {
    page_header_t page_header;
    free_store_t free_space_register;
    frame_store_t frame_register;
} page_t;

typedef struct {
    page_id_t page_id;
    frame_id_t frame_id;
} fid_t;

typedef struct {
    void (*push)();
    void (*fetch)();
} page_cold_store_t;

typedef struct {
    int reserved;
} page_anticache_t;

typedef struct {
    dict_t *page_hot_store;
    page_anticache_t page_anticache;
    page_cold_store_t page_cold_store;

} buffer_manager_t;

typedef enum {
    positioning_first_nomerge,
    positioning_first_merge,
    positioning_smallest_nomerge,
    positioning_smallest_merge,
    positioning_largest_nomerge,
    positioning_largest_merge
} block_positioning;

typedef enum {
    frame_inuse,
    frame_free
} frame_state;

typedef enum {
    type_far_ptr,
    type_near_ptr
} ptr_scope_type;

typedef struct {
    buffer_manager_t *manager;
    page_id_t page_id;
    frame_id_t frame_id;
} block_ptr;

typedef struct {
    buffer_manager_t *manager;
    page_id_t page_id;
    frame_id_t frame_id;
    in_page_ptr zone;
} zone_ptr;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

buffer_manager_t *buffer_manager_create();

block_ptr *buffer_manager_block_alloc(buffer_manager_t *manager, size_t size, size_t nzones);
void buffer_manager_block_free(block_ptr *ptr);

zone_id_t buffer_manager_block_nextzone(block_ptr *ptr);
void buffer_manager_block_setzones(block_ptr *ptr, zone_id_t last_zone);
void buffer_manager_block_rmzone(block_ptr *ptr, zone_id_t zone);

zone_ptr *buffer_manager_block_seek(block_ptr *ptr, zone_id_t zone_id);
zone_ptr *buffer_manager_block_open(block_ptr *ptr);
void buffer_manager_block_next(zone_ptr *ptr);
void buffer_manager_block_close(block_ptr *ptr);

void buffer_manager_zone_read(zone_ptr *zone, void *capture, void (*consumer) (void *capture, const void *data));
void buffer_manager_zone_cpy(zone_ptr *dst, size_t offset, const void *src, size_t num);
void buffer_manager_zone_set(zone_ptr *dst, size_t offset, int value, size_t num);

bool buffer_manager_free(buffer_manager_t *manager);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool page_hot_store_init(buffer_manager_t *manager);

void page_hot_store_set(buffer_manager_t *manager, page_id_t id, void *ptr);

bool page_hot_store_has(buffer_manager_t *manager, page_id_t id);

bool page_hot_store_remove(buffer_manager_t *manager, page_id_t id);

void *page_hot_store_get(buffer_manager_t *manager, page_id_t id);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

// Note: no "flush list"

void page_anticache_init(buffer_manager_t *manager);

void page_anticache_page_new(buffer_manager_t *manager, size_t size);

void page_anticache_page_delete(buffer_manager_t *manager, page_t *page);

void page_anticache_free_list_push(buffer_manager_t *manager, page_t *page);

void page_anticache_heat_list_update(buffer_manager_t *manager, page_t *page);

page_t *page_anticache_free_list_pop(buffer_manager_t *manager);

void page_anticache_page_freeze(buffer_manager_t *manager, page_t *page);

void page_anticache_page_freeze_all(buffer_manager_t *manager);

void page_anticache_free(buffer_manager_t *manager);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void page_cold_store_init(buffer_manager_t *manager);

void page_cold_store_clear(buffer_manager_t *manager);

void *page_cold_store_fetch(buffer_manager_t *manager, page_id_t id);

void page_cold_store_push(buffer_manager_t *manager, page_t *page);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(buffer_manager_t *manager, page_id_t id, size_t size, page_flags flags, size_t free_space, size_t frame_reg);

fid_t *frame_create(page_t *page, block_positioning strategy, size_t element_size);

zone_t *zone_create(buffer_manager_t *manager, page_t *page, fid_t *frame_handle, block_positioning strategy);

bool zone_memcpy(page_t *page, zone_t *zone, const void *data, size_t size);

bool zone_remove(buffer_manager_t *manager, page_t *page, const zone_t *zone);

bool frame_delete(fid_t *frame);

void page_dump(FILE *out, buffer_manager_t *manager, const page_t *page);

bool page_free(page_t *page);



