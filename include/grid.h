#pragma once

#include <frag.h>
#include <tuple.h>
#include <interval.h>
#include <indexes/grid_index.h>

typedef size_t grid_id_t;

typedef struct grid_t {
    frag_t *frag;
    vector_t *attr_idxs_covered;
} grid_t;

typedef struct grids_by_attr_index_elem_t {
    attr_id_t attr_id;
    vector_t *grid_ptrs;
} grids_by_attr_index_elem_t;

typedef struct grid_table_t {
    schema_t *schema; /*<! The schema assigned to this table. Note that this schema is 'logical', i.e., grids
                           have their own schema that might be a subset of this schema with another order on the
                           attributes. The table's schema is used to give a logical structure to a caller. */
    vector_t *grid_ptrs;  /*<! A vector of pointers to elements of type grid_t. Each grid contained in this table is
                           referenced here, and will be freed from here once the table will be disposed. */
    grid_index_t attr_index; /*<! An index that maps attribute ids from this tables schema to a list of pointers
                             to grids that cover at least this attribute in their 'physical' schemata. */
    grid_index_t tuple_index; /*<! An index that maps tuple ids to a list of pointers to grids that cover
                             at least one field of that tuple. */
} grid_table_t;

grid_table_t *gs_grid_table_create(const schema_t *schema);

void gs_grid_table_free(grid_table_t *table);

grid_id_t gs_grid_table_add_grid(grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids,
                                 const interval_t *tuple_id_cover, size_t ntuple_id_cover, enum frag_impl_type_t type);

grid_id_t gs_grid_table_grid_by_field(const grid_table_t *table, attr_id_t attr_id, tuple_id_t tuple_id);

vector_t *gs_grid_table_grids_by_attr(const grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids);

vector_t *gs_grid_table_grids_by_tuples(const grid_table_t *table, const tuple_id_t *tuple_ids, size_t ntuple_ids);

bool gs_grid_table_is_valide(grid_table_t *table);

tuple_t *gs_grid_table_insert(grid_table_t *table, size_t ntuplets);

void gs_grid_table_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit);

void gs_grid_table_grid_print(FILE *file, const grid_table_t *table, grid_id_t grid_id, size_t row_offset, size_t limit);