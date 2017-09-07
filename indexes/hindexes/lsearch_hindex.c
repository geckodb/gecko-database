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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <indexes/hindexes/lsearch_hindex.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct entry_t {
    tuple_id_interval_t interval;
    vector_t *grids;
} entry_t;

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define require_besearch_hindex_tag(index)                                                                             \
    REQUIRE((index->tag == HT_LINEAR_SEARCH), BADTAG);

#define REQUIRE_INSTANCEOF_THIS(index)                                                                                 \
    { REQUIRE_NONNULL(index); REQUIRE_NONNULL(index->extra); require_besearch_hindex_tag(index); }

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline void this_add(struct hindex_t *self, const tuple_id_interval_t *key, const struct grid_t *grid);
static inline void this_remove_interval(struct hindex_t *self, const tuple_id_interval_t *key);
static inline void this_remove_intersec(struct hindex_t *self, tuple_id_t tid);
static inline bool this_contains(const struct hindex_t *self, tuple_id_t tid);
static inline void this_free(struct hindex_t *self);
static inline void this_query(grid_cursor_t *result, const struct hindex_t *self, const tuple_id_t *tid_begin,
                              const tuple_id_t *tid_end);

static inline entry_t *find_interval(vector_t *haystack, const tuple_id_interval_t *needle);
static inline void find_all_by_point(vector_t *result, vector_t *haystack, const tuple_id_t needle);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

hindex_t *lesearch_hindex_create(size_t approx_num_horizontal_partitions, const schema_t *table_schema)
{
    REQUIRE_NONNULL(table_schema);

    hindex_t *result = REQUIRE_MALLOC(sizeof(hindex_t));
    *result = (hindex_t) {
        .tag = HT_LINEAR_SEARCH,

        ._add = this_add,
        ._remove_interval = this_remove_interval,
        ._remove_intersec = this_remove_intersec,
        ._contains = this_contains,
        ._query = this_query,
        ._free = this_free,

        .extra = vector_create(sizeof(entry_t), approx_num_horizontal_partitions),
        .table_schema = table_schema
    };

    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline entry_t *find_interval(vector_t *haystack, const tuple_id_interval_t *needle)
{
    entry_t *it = (entry_t *) haystack->data;
    size_t num_elements = haystack->num_elements;
    while (num_elements--) {
        if (gs_interval_equals((&(it->interval)), needle))
            return it;
        else it++;
    }
    return NULL;
}

static inline void find_all_by_point(vector_t *result, vector_t *haystack, const tuple_id_t needle)
{
    const entry_t *it = (const entry_t *) haystack->data;
    size_t num_elements = haystack->num_elements;
    while (num_elements--) {
        if (needle >= it->interval.begin && needle < it->interval.end) {
            vector_add_all(result, it->grids);
        }
        else it++;
    }
}

static inline void this_add(struct hindex_t *self, const tuple_id_interval_t *key, const struct grid_t *grid)
{
    REQUIRE_INSTANCEOF_THIS(self);
    REQUIRE_NONNULL(key);
    REQUIRE_NONNULL(grid);

    vector_t *vec = self->extra;
    entry_t *result = find_interval(vec, key);
    if (result) {
        vector_add(result->grids, 1, &grid);
    } else {
        entry_t entry = {
                .interval = *key,
                .grids = vector_create(sizeof(struct grid_t *), 16)
        };
        vector_add(entry.grids, 1, &grid);
        vector_add(vec, 1, &entry);
    }
}

static inline void this_query(grid_cursor_t *result, const struct hindex_t *self, const tuple_id_t *tid_begin,
                              const tuple_id_t *tid_end)
{
    REQUIRE_INSTANCEOF_THIS(self);
    REQUIRE_NONNULL(result);
    REQUIRE_NONNULL(tid_begin);
    REQUIRE_NONNULL(tid_end);
    REQUIRE(tid_begin < tid_end, "Corrupted range");
    for (const tuple_id_t *needle = tid_begin; needle != tid_end; needle++) {
        find_all_by_point((vector_t *) result->extra, (vector_t *) self->extra, *needle);
    }
}

static inline void this_remove_interval(struct hindex_t *self, const tuple_id_interval_t *key)
{
    panic(NOTIMPLEMENTED, to_string(this_remove_interval))
}

static inline void this_remove_intersec(struct hindex_t *self, tuple_id_t tid)
{
    panic(NOTIMPLEMENTED, to_string(this_remove_intersec))
}

static inline bool this_contains(const struct hindex_t *self, tuple_id_t tid)
{
    REQUIRE_INSTANCEOF_THIS(self);
    // TODO:...
    panic(NOTIMPLEMENTED, to_string(this_contains))
    return false;
}

static inline bool free_entires(void *capture, void *begin, void *end)
{
    for (entry_t *it = (entry_t *) begin; it < (entry_t *) end; it++) {
        vector_free(it->grids);
    }
    return true;
}

static inline void this_free(struct hindex_t *self)
{
    REQUIRE_INSTANCEOF_THIS(self);
    vector_foreach(self->extra, NULL, free_entires);
    vector_free(self->extra);
}
