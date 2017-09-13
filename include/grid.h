#pragma once

#include <frag.h>
#include <tuple.h>
#include <interval.h>
#include <indexes/vindex.h>
#include <indexes/hindex.h>
#include <containers/freelist.h>
#include <tuple_cursor.h>

typedef size_t grid_id_t;

typedef struct grid_t {
    struct grid_table_t *context; /*<! The grid table in which this grid exists. */
    grid_id_t grid_id; /*<! The id of this grid in context of the grid table. */
    frag_t *frag; /*<! The physical data fragment including the applied physical schema of this grid. */
    dict_t *schema_map_indicies; /*<! An (inverted) index that allows direct access from a table schema attribute to
                                      the associated grid schema attribute via indices mapping. This index returns the
                                      position j in the grid schema attribute list given a position i in the table
                                      schema attribute list, for which hold: the attribute A (in the grid schema) with
                                      position j is the attribute A (in the table schema) with the position i. Hence,
                                      iterating through all i that are mapped into the grid yields all associated j.
                                      Note, the order of attributes might change between the table schema and the grid
                                      schema. */
    vector_t /* of tuple_id_interval_t */ *tuple_ids; /*<! A list of right-open intervals that describe which tuples in the table
                                         are covered in this grid. Intervals [a, b), [c,d ),... in this list are assumed
                                         to be ordered ascending by their lower bound, e.g., a, c for a < c. Further,
                                         the intervals do not overlap. This information is required to translate from
                                         tuple identifiers to per-grid tuplet identifiers and vice versa, since
                                         the intervals in 'tuple_ids' contain a subset of all tuple identifiers in the
                                         table (e.g., [100, 105)). With other words, the union of all intervals describe
                                         a monotonically increasing function that is not continuous. In contrast,
                                         tuplet identifiers per grid is strictly monotonically continuous increasing
                                         (i.e., 0, 1, 2, 3, ...). These information are needed to map a tuple
                                         identifier (e.g., 100) to a tuplet identifier (e.g. 2), and vice versa. */
    tuple_id_interval_t *last_interval_cache; /*<! A nullable pointer into 'tuple_ids' data that is the last accessed interval.
                                         it's used to speedup translation between tuplet and tuple identifier. Caching
                                         the last access avoids searching the interval when a tuplet is request in the
                                         same interval as before. In case the pointer is null, no entry is in the cache.
                                         In case the pointer is non-null, an entry is cached. Note, that's not
                                         guaranteed to find a certain tuplet in this cache. In case the tuplet is not
                                         found here, 'tuple_ids' are searched for a match according some specific
                                         algorithm that is implemented in the manager of this cache. */

    pthread_mutex_t mutex; // TODO: locking a single grid
} grid_t;

typedef enum {
    AT_SEQUENTIAL,  /*<! A call for translation between tuplet and tuple identifier forward iterates through the
                         the intervals of tuple identifiers coverd by a grid. If a tuple is not found in the
                         last (cached) interval, it is guaranteed that the tuple is not in a preceding interval
                         w.r.t. to the cached interval. With other words, if a tuple is not in the (cached) interval
                         it must be in one of the successors of the cached one. Sequential access is typical for
                         full table scans. */
    AT_RANDOM       /*<! A call for translation between tuplet and tuple identifier where there are no guarantees
                         on the order in which intervals are accessed. With other words, when running a random
                         access type, intervals between succeeding calls for translation are likely to change. Hence,
                         if the tuple is not in the last (cached) interval, the entire list of intervals must be
                         searched. Random access is typical for index scans. */
} access_type;

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
    size_t num_tuples; /*<! The number of tuples in this table. Note: it's guaranteed that the sequence of
                            tuple identifiers from 0 to num_tuples - 1 is strictly monotonically continuous increasing.
                            With other words, each tuple identifier in the right open interval [0, num_tuples) is
                            accessible. However, it's neither guaranteed that a tuple associated with an identifier in
                            this interval is not marked as 'deleted' nor that the tuple data is initialized. */
} grid_table_t;

grid_table_t *gs_grid_table_create(const schema_t *schema, size_t approx_num_horizontal_partitions);

void gs_grid_table_free(grid_table_t *table);

void gs_grid_free(grid_t * grid);

const char *gs_grid_table_name(const grid_table_t *table);

grid_id_t gs_grid_table_add_grid(grid_table_t *table, const attr_id_t *attr_ids_covered, size_t nattr_ids_covered,
                                 const tuple_id_interval_t *tuple_ids_covered, size_t ntuple_ids_covered, enum frag_impl_type_t type);

const freelist_t *gs_grid_table_freelist(const struct grid_table_t *table);

grid_cursor_t *gs_grid_table_grid_find(const grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids,
                                  const tuple_id_t *tuple_ids, size_t ntuple_ids);

grid_table_t *gs_grid_table_melt(enum frag_impl_type_t type, const grid_table_t *src_table, const tuple_id_t *tuple_ids,
                           size_t ntuple_ids, const attr_id_t *attr_ids, size_t nattr_ids);

const attr_t *gs_grid_table_attr_by_id(const grid_table_t *table, attr_id_t id);

const char *gs_grid_table_attr_name_by_id(const grid_table_t *table, attr_id_t id);

size_t gs_grid_table_num_of_attributes(const grid_table_t *table);

size_t gs_grid_table_num_of_tuples(const grid_table_t *table);

size_t gs_grid_table_num_of_grids(const grid_table_t *table);

const attr_id_t *gs_grid_table_attr_id_to_frag_attr_id(const grid_t *grid, attr_id_t table_attr_id);

const grid_t *gs_grid_by_id(const grid_table_t *table, grid_id_t id);

size_t gs_grid_num_of_attributes(const grid_t *grid);

vector_t *gs_grid_table_grids_by_attr(const grid_table_t *table, const attr_id_t *attr_ids, size_t nattr_ids);

vector_t *gs_grid_table_grids_by_tuples(const grid_table_t *table, const tuple_id_t *tuple_ids, size_t ntuple_ids);

bool gs_grid_table_is_valide(grid_table_t *table);

void gs_grid_table_insert(tuple_cursor_t *resultset, grid_table_t *table, size_t ntuplets);

void gs_grid_print(FILE *file, const grid_table_t *table, grid_id_t grid_id, size_t row_offset, size_t limit);

void gs_grid_table_grid_list_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit);

void gs_grid_table_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit);

void gs_grid_table_structure_print(FILE *file, const grid_table_t *table, size_t row_offset, size_t limit);

static inline int interval_tuple_id_comp_by_element(const void *needle, const void *element)
{
    tuple_id_t tuple_id = *(const tuple_id_t *) needle;
    const tuple_id_interval_t *interval = element;
    return (GS_INTERVAL_CONTAINS(interval, tuple_id) ? 0 : (tuple_id < interval->begin ? - 1 : + 1));
}

static inline tuplet_id_t gs_grid_global_to_local(grid_t *grid, tuple_id_t tuple_id, access_type type)
{
    // TODO: apply a bunch of strategy alternatives here. For instance, one can buffer the distances for a given
    // Cursor and all its processors. Then the translation does not require to scan the entire interval list in order
    // to provide the tuplet id. Also, one can run binary search and linear search probably multi-threaded...

    // The tuplet identifier that is mapped in this grid to the given 'tuplet_id'
    tuplet_id_t result = 0;

    // Determine where to start in the interval list. The last position was cached, so start here if possible
    tuple_id_interval_t *cursor = (grid->last_interval_cache != NULL ? grid->last_interval_cache : grid->tuple_ids->data);

    // Determine the end of the interval list. If cursor reaches this end, the tuple is not mapped into this grid.
    // Clearly, if this case happens, there is an interval error since a request to this function requires
    // a valid coverage of the given 'tuple_id' in this grid
    const tuple_id_interval_t *end = vector_end(grid->tuple_ids);

    if (!GS_INTERVAL_CONTAINS(cursor, tuple_id)) {
        switch (type) {
            case AT_SEQUENTIAL:
                // seek to interval that contains the tuple, move cursor to successor since its guaranteed not in the
                // current one
                for (cursor++; cursor < end && !GS_INTERVAL_CONTAINS(cursor, tuple_id); cursor++);
                break;
            case AT_RANDOM:
                // Assert that interval list is sorted. If sort state is not cached,
                // re-evaluate the state and check again
                assert (vector_issorted(grid->tuple_ids, CCP_USECACHE, NULL) ||
                        vector_issorted(grid->tuple_ids, CCP_IGNORECACHE, gs_interval_tuple_id_comp_by_lower_bound));

                // TODO: apply evolutionary algorithm here to find choice of alternatives once alternatives exists
                // Alternatives besides bsearch might be linear search w/o multi-threading from cache or start/end, ...

                // Perform binary search. Note here that this only works, because we exploit an assumption on the
                // intervals contained in the list: intervals do not overlap and the needle can be used to state whether
                // a lower or higher interval must be considered during search if the needle is not contained in the
                // current interval.
                cursor = vector_bsearch(grid->tuple_ids, &tuple_id,
                                        gs_interval_tuple_id_comp_by_lower_bound, interval_tuple_id_comp_by_element);
                break;
            default: panic(BADBRANCH, grid);
        }
    }

    panic_if((cursor == end), "Internal error: mapping of tuple '%u' is not resolvable", tuple_id);

    // TODO: Cache this!
    // calculate the number of tuplets that fall into preceding intervals
    for (const tuple_id_interval_t *it = vector_begin(grid->tuple_ids); it < end; it++) {
        result += (tuple_id >= it->begin) ? gs_interval_get_span(it) : 0;
    }
    // calculate the exact identifier for the given tuple in the 'cursor' interval
    result += (tuple_id - cursor->end);

    // update cache
    grid->last_interval_cache = cursor;

    return result;
}

static inline tuple_id_t gs_grid_local_to_global(grid_t *grid, tuplet_id_t tuplet_id)
{
    assert (tuplet_id < grid->frag->ntuplets);

    // TODO: Cache this!
    tuple_id_t num_tuple_covereed = 0;

    const tuple_id_interval_t *it = vector_begin(grid->tuple_ids);
    // since tuples in the interval list maps bidirectional to tuplets, it holds with all other assumptions on the
    // interval list and the construction of the mapping: in the order-preserving union of all intervals,
    // the i-th tuple is mapped to the tuplet is i.

    // sum up the number of tuples covered in each interval until this sum exceeds 'tuplet_id'
    for (; num_tuple_covereed <= tuplet_id; num_tuple_covereed += gs_interval_get_span(it), it++);

    // return the tuple identifier that is mapped to the tuplet id
    return (it->end - (num_tuple_covereed - tuplet_id) - 1);
}