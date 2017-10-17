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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <indexes/hindexes/gs_lsearch_hindex.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct entry_t {
    gs_tuple_id_interval_t interval;
    gs_vec_t *grids;
} entry_t;

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define require_besearch_hindex_tag(index)                                                                             \
    REQUIRE((index->tag == HT_LINEAR_SEARCH), BADTAG);

#define REQUIRE_INSTANCEOF_THIS(index)                                                                                 \
    { GS_REQUIRE_NONNULL(index); GS_REQUIRE_NONNULL(index->extra); require_besearch_hindex_tag(index); }

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

 void this_add(struct gs_hindex_t *self, const gs_tuple_id_interval_t *key, const struct gs_grid_t *grid);
 void this_remove_interval(struct gs_hindex_t *self, const gs_tuple_id_interval_t *key);
 void this_remove_intersec(struct gs_hindex_t *self, gs_tuple_id_t tid);
 bool this_contains(const struct gs_hindex_t *self, gs_tuple_id_t tid);
 void this_delete(struct gs_hindex_t *self);
 void this_query(gs_grid_cursor_t *result, const struct gs_hindex_t *self, const gs_tuple_id_t *tid_begin,
                              const gs_tuple_id_t *tid_end);
 gs_tuple_id_t this_minbegin(struct gs_hindex_t *self);
 gs_tuple_id_t this_maxend(struct gs_hindex_t *self);
 gs_tuple_id_t bounds(struct gs_hindex_t *self, bool begin);

 entry_t *find_interval(gs_vec_t *haystack, const gs_tuple_id_interval_t *needle);
 void find_all_by_point(gs_vec_t *result, gs_vec_t *haystack, const gs_tuple_id_t needle);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_hindex_t *gs_lesearch_hindex_new(size_t approx_num_horizontal_partitions, const gs_schema_t *table_schema)
{
    GS_REQUIRE_NONNULL(table_schema);

    gs_hindex_t *result = GS_REQUIRE_MALLOC(sizeof(gs_hindex_t));
    *result = (gs_hindex_t) {
        .tag = HT_LINEAR_SEARCH,

        ._add = this_add,
        ._remove_interval = this_remove_interval,
        ._remove_intersec = this_remove_intersec,
        ._contains = this_contains,
        ._query = this_query,
        ._delete = this_delete,
        ._minbegin = this_minbegin,
        ._maxend = this_maxend,

        .extra = gs_vec_new(sizeof(entry_t), approx_num_horizontal_partitions),
        .table_schema = table_schema
    };

    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

 entry_t *find_interval(gs_vec_t *haystack, const gs_tuple_id_interval_t *needle)
{
    entry_t *it = (entry_t *) haystack->data;
    size_t num_elements = haystack->num_elements;
    while (num_elements--) {
        if (INTERVAL_EQUALS((&(it->interval)), needle))
            return it;
        else it++;
    }
    return NULL;
}

 void find_all_by_point(gs_vec_t *result, gs_vec_t *haystack, const gs_tuple_id_t needle)
{
    const entry_t *it = (const entry_t *) haystack->data;
    size_t num_elements = haystack->num_elements;
    while (num_elements--) {
        if (INTERVAL_CONTAINS((&it->interval), needle)) {
            gs_vec_add_all(result, it->grids);
        }
        it++;
    }
}

 void this_add(struct gs_hindex_t *self, const gs_tuple_id_interval_t *key, const struct gs_grid_t *grid)
{
    REQUIRE_INSTANCEOF_THIS(self);
    GS_REQUIRE_NONNULL(key);
    GS_REQUIRE_NONNULL(grid);

    gs_vec_t *vec = self->extra;
    entry_t *result = find_interval(vec, key);
    if (result) {
        gs_vec_pushback(result->grids, 1, &grid);
    } else {
        entry_t entry = {
                .interval = *key,
                .grids = gs_vec_new(sizeof(struct gs_grid_t *), 16)
        };
        gs_vec_pushback(entry.grids, 1, &grid);
        gs_vec_pushback(vec, 1, &entry);
    }
}

 void this_query(gs_grid_cursor_t *result, const struct gs_hindex_t *self, const gs_tuple_id_t *tid_begin,
                              const gs_tuple_id_t *tid_end)
{
    REQUIRE_INSTANCEOF_THIS(self);
    GS_REQUIRE_NONNULL(result);
    GS_REQUIRE_NONNULL(tid_begin);
    GS_REQUIRE_NONNULL(tid_end);
    REQUIRE(tid_begin < tid_end, "Corrupted range");
    for (const gs_tuple_id_t *needle = tid_begin; needle != tid_end; needle++) {
        find_all_by_point((gs_vec_t *) result->extra, (gs_vec_t *) self->extra, *needle);
    }
}

 void this_remove_interval(struct gs_hindex_t *self, const gs_tuple_id_interval_t *key)
{
    panic(NOTIMPLEMENTED, to_string(this_remove_interval))
}

 void this_remove_intersec(struct gs_hindex_t *self, gs_tuple_id_t tid)
{
    panic(NOTIMPLEMENTED, to_string(this_remove_intersec))
}

 bool this_contains(const struct gs_hindex_t *self, gs_tuple_id_t tid)
{
    REQUIRE_INSTANCEOF_THIS(self);
    // TODO:...
    panic(NOTIMPLEMENTED, to_string(this_contains))
    return false;
}

 bool free_entires(void *capture, void *begin, void *end)
{
    for (entry_t *it = (entry_t *) begin; it < (entry_t *) end; it++) {
        gs_vec_free(it->grids);
    }
    return true;
}

 void this_delete(struct gs_hindex_t *self)
{
    REQUIRE_INSTANCEOF_THIS(self);
    gs_vec_foreach(self->extra, NULL, free_entires);
    gs_vec_free(self->extra);
}

 gs_tuple_id_t bounds(struct gs_hindex_t *self, bool begin)
{
    REQUIRE_INSTANCEOF_THIS(self);
    gs_tuple_id_t tuple = begin ? INT_MAX : INT_MIN;
    entry_t *it = gs_vec_begin((gs_vec_t *) self->extra);
    size_t num  = gs_vec_length((gs_vec_t *) self->extra);
    while (num--) { tuple = begin ? min(tuple, (it++)->interval.begin) : max(tuple, (it++)->interval.end); }
    return tuple;
}

 gs_tuple_id_t this_minbegin(struct gs_hindex_t *self)
{
    return bounds(self, true);
}

 gs_tuple_id_t this_maxend(struct gs_hindex_t *self)
{
    return bounds(self, false);
}