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

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef uint8_t  u8;
typedef size_t offset_t;
typedef uint32_t page_id_t;

typedef struct {
    offset_t begin, end;
} memory_range_t;

typedef struct {
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

typedef struct {
    page_id_t page_id;
    size_t free_space;
    struct {
        u8 is_dirty  : 1;
        u8 is_fixed  : 1;
        u8 is_locked : 1;
    } flags;
} page_header_t;

typedef struct {
    size_t list_max, list_len;
} stack_header_t;

typedef struct {
    zone_ptr start;
    size_t elem_size;
    size_t elem_capacity;
} vframe_t;

typedef struct {
    zone_ptr prev, next;
} zone_header_t;

typedef struct {
    page_header_t page_header;
    stack_header_t free_space_register, frame_register;
} page_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(page_id_t id, size_t size, page_flags flags, size_t free_space, size_t frame_reg);

bool page_free(page_t *page);



