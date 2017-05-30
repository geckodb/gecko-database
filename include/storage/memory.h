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

#define FORCE_PACKING        __attribute__((__packed__))

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;

typedef u32 page_id_t;
typedef u32 frame_id_t;

typedef size_t offset_t;

typedef struct {
    offset_t begin, end;
} range_t;

typedef struct FORCE_PACKING {
    struct {
        page_id_t is_far_ptr    : 1;
        page_id_t page_id       : 31;
    };
    offset_t offset;
} zone_ptr;

typedef enum {
    page_flag_dirty = 1 << 1,
    page_flag_fixed = 1 << 2,
    page_flag_locked = 1 << 3
} page_flags;

typedef struct FORCE_PACKING {
    page_id_t page_id;
    size_t page_size, free_space;
    struct {
        u8 is_dirty  : 1;
        u8 is_fixed  : 1;
        u8 is_locked : 1;
    } flags;
} page_header_t;

typedef struct FORCE_PACKING {
    u32 list_max, list_len;
} free_store_t;

typedef struct FORCE_PACKING {
    u32 max_num_frames, in_use_num, free_list_len;
} frame_store_t;

typedef struct FORCE_PACKING {
    zone_ptr start;
    size_t elem_size;
    size_t elem_capacity;
} frame_t;

typedef struct FORCE_PACKING {
    zone_ptr prev, next;
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

typedef enum {
    positioning_first_nomerge,
    positioning_first_merge,
    positioning_smallest_nomerge,
    positioning_smallest_merge,
    positioning_largest_nomerge,
    positioning_largest_merge
} block_positioning;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(page_id_t id, size_t size, page_flags flags, size_t free_space, size_t frame_reg);

fid_t *frame_create(page_t *page, block_positioning strategy, size_t element_size, size_t element_capacity);

bool frame_delete(fid_t *frame);

void page_dump(FILE *out, const page_t *page);

bool page_free(page_t *page);



