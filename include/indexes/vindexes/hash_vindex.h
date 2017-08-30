#pragma once

#include <indexes/vindex.h>

struct schema_t;

vindex_t *hash_vindex_create(size_t key_size, size_t num_init_slots);