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

#include <indexes/hindex.h>
#include <grid_cursor.h>
#include <schema.h>
#include <grid.h>
#include <tuplet_field.h>
#include <containers/dicts/hash_table.h>

typedef struct print_tuple_t
{
    grid_id_t grid;
    tuple_id_t begin, end;
} print_tuple_t;

void hindex_delete(struct hindex_t *index)
{
    DELEGATE_CALL(index, _delete);
}

void hindex_add(struct hindex_t *index, const tuple_id_interval_t *key, const struct grid_t *grid)
{
    REQUIRE_NONNULL(index)
    REQUIRE_NONNULL(key)
    index->bounds.begin = min(index->bounds.begin, key->begin);
    index->bounds.end   = max(index->bounds.end, key->end);
    DELEGATE_CALL_WARGS(index, _add, key, grid);
}

void hindex_remove(struct hindex_t *index, const tuple_id_interval_t *key)
{
    REQUIRE_NONNULL(index)
    REQUIRE_NONNULL(key)
    DELEGATE_CALL_WARGS(index, _remove_interval, key);
    index->bounds.begin = (key->begin == index->bounds.begin)? DELEGATE_CALL(index, _minbegin) : index->bounds.begin;
    index->bounds.end = (key->end == index->bounds.end)? DELEGATE_CALL(index, _maxend) : index->bounds.end;
}

void hindex_remove_having(struct hindex_t *index, tuple_id_t tid)
{
    DELEGATE_CALL_WARGS(index, _remove_intersec, tid);
}

bool hindex_contains(const struct hindex_t *index, tuple_id_t tid)
{
    return DELEGATE_CALL_WARGS(index, _contains, tid);
}

grid_cursor_t *hindex_query(const struct hindex_t *index, const tuple_id_t *tid_begin,
                            const tuple_id_t *tid_end)
{
    REQUIRE_NONNULL(tid_begin);
    REQUIRE_NONNULL(tid_end);
    REQUIRE(tid_begin < tid_end, "Corrupted range");
    size_t approx_result_capacity = ((tid_end - tid_begin) * schema_num_attributes(index->table_schema));
    grid_cursor_t *result = grid_cursor_new(approx_result_capacity);
    index->_query(result, index, tid_begin, tid_end);
    return result;
}

const struct grid_t *hindex_read(grid_cursor_t *result_set)
{
    return grid_cursor_next(result_set);
}

void hindex_close(grid_cursor_t *result_set)
{
    grid_cursor_delete(result_set);
}

void hindex_bounds(tuple_id_interval_t *bounds, const hindex_t *index)
{
    REQUIRE_NONNULL(bounds);
    REQUIRE_NONNULL(index);
    bounds->begin = index->bounds.begin;
    bounds->end = index->bounds.end;
}

void hindex_print(FILE *file, const hindex_t *index)
{
    schema_t *print_schema = schema_new("ad hoc info");
    attr_create_tupleid("begin", print_schema);
    attr_create_tupleid("end", print_schema);
    attr_create_gridid("grid id", print_schema);
    frag_t *frag = frag_new(print_schema, 1, FIT_HOST_NSM_VM);
    size_t dist = index->bounds.end - index->bounds.begin;
    tuple_id_t *ids = REQUIRE_MALLOC(dist * sizeof(tuple_id_t));
    for (tuple_id_t i = index->bounds.begin; i < index->bounds.end; i++) {
        ids[i] = i;
    }
    grid_cursor_t *cursor = hindex_query(index, ids, ids + dist);
    tuplet_field_t field;
    tuplet_t tuplet;

    /* dedup entries by hand */
    dict_t *dict = hash_table_new_jenkins(sizeof(print_tuple_t),
                                          sizeof(bool), 1.5f * grid_cursor_numelem(cursor), 1.7f, 0.75f);

    for (struct grid_t *grid = grid_cursor_next(cursor); grid != NULL; grid = grid_cursor_next(NULL)) {
       size_t interval_idx = 0;
       const tuple_id_interval_t *interval = vec_at(grid->tuple_ids, interval_idx); bool dummy;
       print_tuple_t t = { .grid = grid->grid_id, .begin = interval->begin, .end = interval->end };
       if (!dict_contains_key(dict, &t)) {
           frag_insert(&tuplet, frag, 1);
           tuplet_field_open(&field, &tuplet);
           tuplet_field_write(&field, &interval->begin, true);
           tuplet_field_write(&field, &interval->end, true);
           tuplet_field_write(&field, &grid->grid_id, true);
           dict_put(dict, &t, &dummy);
       }
    }

    free(ids);
    dict_delete(dict);
    frag_print(file, frag, 0, INT_MAX);
    grid_cursor_delete(cursor);
    frag_delete(frag);
    schema_delete(print_schema);
}