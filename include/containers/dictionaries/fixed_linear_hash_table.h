// An implementation of a open addressing hash table data structure with linear probing
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

//
// The following provides an overview on design decisions made for this implementation
//
// Addressing mode:                 open addressing
// Collision resolving:             linear probing
// Reorganization strategy:         allocate additional slot list, rehash old list, free old list
// Reorganization request:          Eager & blocking
// Point queries:                   yes
// Range queries:                   no
// Thread-safe:                     yes, lock-based via explicit call to lock/unlock functions
// Compression:                     no
// Parallel computing mode:         task parallelism
// Slot list:                       dynamically allocated continuous memory block
// Slot memory:                     virtual memory
// Empty slot indicator:            dedicated value for key entry (i.e., max value of key type)
// Key-value pairs:                 embedded in slot list, accessed via offset
// Key-value pair memory:           virtual memory
// Get result:                      const pointer into key-value pair (i.e., no memcpy)
// 'Get' CPU cache efficiency:      efficient (no random access)
// 'Put' CPU cache efficiency:      efficient (no random access)
// Get request:                     Eager & blocking
// Put request:                     Eager & blocking
// SIMD:                            no
// Use of CPU prefetch hints:       no
// Use of CPU branch pred. hints:   no

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <defs.h>
#include <hash.h>
#include <containers/dict.h>

typedef struct {
    size_t num_put_calls, num_collisions, num_locks, num_rebuilds, num_put_slotsearch, num_updates,
           num_get_foundkey, num_get_slotdisplaced, num_get_nosuchkey_fullsearch, num_get_nosuchkey,
            num_test_slot, num_slot_get_key, num_slot_get_value, num_remove_key, num_remove_slotdisplaced,
            num_remove_nosuchkey_fullsearch, num_remove_nosuchkey;
} counters_t;

typedef struct
{
    size_t num_slots_free, num_slots_inuse;
    size_t overhead_size, user_data_size;
    double load_factor;
    counters_t counters;
} linear_hash_table_info_t;

typedef struct {
    const void *capture;
    hash_code_fn_t hash_code;
} hash_function_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

dict_t *hash_table_create(const hash_function_t *hash_function, size_t key_size, size_t elem_size,
                          size_t num_slots, float grow_factor, float max_load_factor);

dict_t *hash_table_create_ex(const hash_function_t *hash_function, size_t key_size, size_t elem_size,
                          size_t num_slots, float grow_factor, float max_load_factor,
                          bool (*equals)(const void *key_lhs, const void *key_rhs),
                          void (*cleanup)(void *key, void *value), bool key_is_str);

void hash_table_lock(dict_t *dict);
void hash_table_unlock(dict_t *dict);
bool hash_table_free(dict_t *dict);
void hash_reset_counters(dict_t *dict);
void hash_table_info(dict_t *dict, linear_hash_table_info_t *info);


