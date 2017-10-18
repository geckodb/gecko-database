// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
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

#include <gs.h>
#include <containers/gs_vec.h>


// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define GS_HASH_BUCKET_CAP 10

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_hash_t gs_hash_t;


// typedef struct gs_hash_it_t gs_hash_it_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

__BEGIN_DECLS

GS_DECLARE(gs_status_t) gs_hash_create(gs_hash_t **hash, size_t num_buckets, gs_comp_func_t key_comp);

GS_DECLARE(gs_status_t) gs_hash_create_ex(gs_hash_t **hash, size_t num_buckets, gs_hash_code_fn_t hash_func, gs_comp_func_t key_comp);

GS_DECLARE(gs_status_t) gs_hash_dispose(gs_hash_t *hash);

GS_DECLARE(gs_status_t) gs_hash_set(gs_hash_t *hash, void *key, size_t key_size, void *val, gs_comp_func_t key_comp);

GS_DECLARE(gs_status_t) gs_hash_set_ex(gs_hash_t *hash, void *keys, size_t key_size, void *vals, size_t num_elems, gs_comp_func_t key_comp);

GS_DECLARE(gs_status_t) gs_hash_unset_ex(gs_hash_t *hash, void *keys, size_t key_size, size_t num_elems);

GS_DECLARE(void *) gs_hash_get(const gs_hash_t *hash, void *key, size_t key_size);

GS_DECLARE(gs_status_t) gs_hash_get_ex(gs_vec_t *result, const gs_hash_t *hash, void *keys, size_t key_size, size_t num_elems);

// GS_DECLARE(gs_status_t) gs_hash_open(gs_hash_it_t **it, const gs_hash_t *hash);

// GS_DECLARE(gs_status_t) gs_hash_next(gs_hash_it_t *it, const void **key, const void **value);

// GS_DECLARE(gs_status_t) gs_hash_close(gs_hash_it_t *it);

__END_DECLS