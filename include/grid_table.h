#pragma once

#include <stdinc.h>
#include <attr.h>
#include <tuple.h>
#include <frag.h>

struct grid_table_t;
struct grid_t;

enum column_index_tag {
    CIT_HASH_TABLE
};

enum row_index_tag {
    RIT_HASH_TABLE
};

typedef struct column_index_t {
    enum column_index_tag tag;
    void *extra;

    vector_t* (*_query)(struct column_index_t *self, struct grid_table_t *table, attr_t *attrs, size_t num_attrs);
    void (*_free_result)(struct column_index_t *self, vector_t *result);
    void (*_update)(struct column_index_t *self, attr_t *attrs, size_t num_attrs, const struct grid_t *grid);
    void (*_remove)(struct column_index_t *self, attr_t *attrs, size_t num_attrs);
} column_index_t;

typedef struct row_index_t {
    enum row_index_tag tag;
    void *extra;

    vector_t* (*_query)(struct row_index_t *self, struct row_index_t *table, tuple_id_t *tids, size_t num_tids);
    void (*_free_result)(struct row_index_t *self, vector_t *result);
    void (*_update)(struct row_index_t *self, tuple_id_t *tids, size_t num_tids, const struct grid_t *grid);
    void (*_remove)(struct row_index_t *self, tuple_id_t *tids, size_t num_tids);
} row_index_t;

typedef struct grid_t {
    frag_t *frag;
    vector_t *tuple_id_mapping;
} grid_t;

typedef struct grid_table_t {
    column_index_t *column_index;
    row_index_t *row_index;
    vector_t *grids;
} grid_table_t;



grid_t *gs_grid_create(schema_t *schema, size_t tuplet_capacity, enum frag_impl_type_t type);

void gs_grid_free(grid_t *grid);

tuplet_t *gs_grid_insert(grid_t *grid, const tuple_id_t *tuple_ids, size_t num_tuples);

const tuple_id_t *gs_grid_covered_tuples(const grid_t *grid);

size_t gs_grid_num_tuplets(const grid_t *grid);

const schema_t *gs_grid_schema(const grid_t *grid);