#pragma once

#include <indexes/hindex.h>

hindex_t *lesearch_hindex_create(size_t approx_num_horizontal_partitions, const schema_t *table_schema);