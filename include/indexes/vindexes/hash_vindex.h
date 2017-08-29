#pragma once

#include <indexes/vindex.h>

struct schema_t;

vindex_t *hash_vindex_create(size_t key_size, size_t num_init_slots,
                             bool (*equals)(const void *key_lhs, const void *key_rhs),
                             void (*cleanup)(void *key, void *value));