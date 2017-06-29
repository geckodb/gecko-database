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
#include <containers/list.h>

#define __force_packing__                   __attribute__((__packed__))

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define HOTSTORE_INITCAP                       100
#define HOTSTORE_GROW_FACTOR                 2.00f
#define HOTSTORE_MAX_FILL_FAC                0.75f

#define ANTICACHE_HOTSTORELIST_INITCAP         100
#define ANTICACHE_HOTSTORELIST_GROW_FACTOR    1.7f
#define ANTICACHE_COLDSTORELIST_INITCAP        100
#define ANTICACHE_COLDSTORELIST_GROW_FACTOR   1.7f
#define ANTICACHE_PAGEID_FREELIST_INITCAP      100
#define ANTICACHE_PAGEID_FREELIST_GROW_FACTOR 1.7f

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef uint32_t       page_id_t;
typedef uint32_t       lane_id_t;
typedef uint32_t       zone_id_t;

typedef size_t         offset_t;

typedef struct __force_packing__ {
        uint32_t       list_max;
        uint32_t       list_len;
} freespace_reg_t;

typedef struct __force_packing__ {
        uint64_t       created;
        uint64_t       read;
        uint64_t       write;
        uint64_t       num_reads;
        uint64_t       num_writes;
} access_stats_t;

typedef struct __force_packing__ {
        uint32_t       lane_num_limit;
        uint32_t       in_use_num;
        uint32_t       free_list_len;
} lane_reg_t;

typedef struct __force_packing__ {
        page_id_t      id;
        size_t         size;
        size_t         freespace;
        access_stats_t statistics;
        struct {
            uint8_t is_dirty  : 1;
            uint8_t is_fixed  : 1;
            uint8_t is_locked : 1;
        } flags;
} page_header_t;

typedef struct {
        page_header_t   header;
        freespace_reg_t freespace_reg;
        lane_reg_t      lane_reg;
} page_t;

typedef struct {
        void          (*push)();
        page_t *      (*fetch)(page_id_t id);
} cold_store_t;

typedef struct {
        page_id_t     next_page_id;
        vector_t *    free_page_ids_stack;  /* page ids that are free to be recycled */

        dict_t *      hot_store;
        list_t *      hot_store_page_ids;

        cold_store_t  cold_store;
        vector_t *    cold_store_page_ids;  /* all page ids that are in use who live in cold-store */
} page_anticache_t;

typedef struct {
        size_t        page_size;
        size_t        free_space_reg_capacity;
        size_t        lane_reg_capacity;
} buffer_manager_config_t;

typedef struct {
        size_t                  max_size_hot_store;
        page_anticache_t        page_anticache;
        buffer_manager_config_t config;
} anti_buf_t;

typedef enum {
        block_state_closed,
        block_state_opened
} block_state;

typedef struct __force_packing__ {
        struct __force_packing__ {
            page_id_t is_far_ptr    : 1;
            page_id_t page_id       : 31;
        };
        struct __force_packing__ {
            offset_t  target_type_bit_0    : 1;
            offset_t  target_type_bit_1    : 1;
            offset_t  offset               : 62;
        };
} in_page_ptr;

typedef struct __force_packing__ {
        in_page_ptr prev, next;
        access_stats_t statistics;
} zone_t;

typedef enum {
        positioning_first_nomerge,
        positioning_first_merge,
        positioning_smallest_nomerge,
        positioning_smallest_merge,
        positioning_largest_nomerge,
        positioning_largest_merge
} block_pos;

typedef struct {
        anti_buf_t *  manager;
        page_id_t     page_id;
        lane_id_t     lane_id;
        zone_t *      zone;
        block_state   state;
} cursor_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

anti_buf_t *
buf_create(
        size_t        page_size,
        size_t        freespace_reg_capacity,
        size_t        lane_reg_capacity,
        size_t        hotstore_size_limit
);

bool
buf_free(
        anti_buf_t *  buf
);

cursor_t *
buf_alloc(
        anti_buf_t *  buf,
        size_t        size,
        size_t        nlanes,
        block_pos     strat
);

void
buf_release(
        cursor_t *    cur
);

void
buf_open(
        cursor_t *    cur
);

bool
buf_next(
        cursor_t *    cur
);

void
buf_close(
        cursor_t *    cur
);

void
buf_read(
        cursor_t *    cur,
        void *        capture,
        void          (*consumer)(void *capture, const void *data)
);

void
buf_memcpy(
        cursor_t *    dst,
        size_t        offset,
        const void *  src,
        size_t        size
);

void
buf_dump(
        FILE *        out,
        anti_buf_t *  buf,
        bool          hex_view
);
