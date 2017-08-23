#pragma once

#include <frag.h>
#include <tuple.h>
#include <interval.h>
#include <indexes/vindex.h>
#include <indexes/hindex.h>
#include <containers/freelist.h>
#include <resultset.h>

typedef size_t grid_id_t;

typedef struct grid_t {
    struct grid_table_t *context; /*<! The grid table in which this grid exists. */
    frag_t *frag; /*<! The physical data fragment including the applied physical schema of this grid. */
    dict_t *schema_map_indicies; /*<! An (inverted) index that allows direct access from a table schema attribute to
                                      the associated grid schema attribute via indices mapping. This index returns the
                                      position j in the grid schema attribute list given a position i in the table
                                      schema attribute list, for which hold: the attribute A (in the grid schema) with
                                      position j is the attribute A (in the table schema) with the position i. Hence,
                                      iterating through all i that are mapped into the grid yields all associated j.
                                      Note, the order of attributes might change between the table schema and the grid
                                      schema. */
    pthread_mutex_t mutex; // TODO: locking a single grid
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
    vindex_t *schema_cover; /*<! An index that maps attribute ids from this tables schema to a list of pointers
                             to grids that cover at least this attribute in their 'physical' schemata. */
    hindex_t *tuple_cover; /*<! An index that maps ranges of tuple ids to a list of pointers to grids that
                             contribute to at least one of the tuples in the range. */
    freelist_t tuple_id_freelist; /*<! Tuple identifiers that can be re-used. A tuple identifier will be added to this
                                      list after the physical tuple associated with the identifier was removed from
                                      the grid table. A strictly auto-increasing number that provides a new tuple
                                      identifier that never was used before is also stored here. */
} grid_table_t;

grid_table_t *gs_grid_table_create(const schema_t *schema, size_t approx_num_horizontal_partitions);

void gs_grid_table_free(grid_table_t *table);

const char *gs_grid_table_name(const grid_table_t *table);

grid_id_t gs_grid_table_add_grid(grid_table_t *table, const attr_id_t *attr_ids_covered, size_t nattr_ids_covered,
                                 const tuple_id_interval_t *tuple_ids_covered, size_t ntuple_ids_covered, enum frag_impl_type_t type);

const freelist_t *gs_grid_table_freelist(const struct grid_table_t *table);

grid_id_t gs_grid_table_grid_by_field(const grid_table_t *table, attr_id_t attr_id, tuple_id_t tuple_id);

attr_id_t gs_grid_table_attr_id_to_frag_attr_id(const grid_t *grid, attr_id_t table_attr_id);

const grid_t *gs_grid_by_id(const grid_table_t *table, grid_id_t id);

vector_t *gs_grid_table_grids_by_attr(const grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids);

vector_t *gs_grid_table_grids_by_tuples(const grid_table_t *table, const tuple_id_t *tuple_ids, size_t ntuple_ids);

bool gs_grid_table_is_valide(grid_table_t *table);

void gs_grid_table_insert(resultset_t *resultset, grid_table_t *table, size_t ntuplets);

void gs_grid_table_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit);

void gs_grid_table_grid_print(FILE *file, const grid_table_t *table, grid_id_t grid_id, size_t row_offset, size_t limit);