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

#include <indexes/gs_hindex.h>
#include <gs_grid_cursor.h>
#include <gs_schema.h>
#include <gs_grid.h>
#include <gs_tuplet_field.h>
#include <apr_strings.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct print_tuple_t
{
    gs_grid_id_t grid;
    gs_tuple_id_t begin, end;
} print_tuple_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_hindex_delete(struct gs_hindex_t *index)
{
    DELEGATE_CALL(index, _delete);
}

void gs_hindex_add(struct gs_hindex_t *index, const gs_tuple_id_interval_t *key, const struct gs_grid_t *grid)
{
    GS_REQUIRE_NONNULL(index)
    GS_REQUIRE_NONNULL(key)
    index->bounds.begin = min(index->bounds.begin, key->begin);
    index->bounds.end   = max(index->bounds.end, key->end);
    DELEGATE_CALL_WARGS(index, _add, key, grid);
}

void gs_hindex_remove(struct gs_hindex_t *index, const gs_tuple_id_interval_t *key)
{
    GS_REQUIRE_NONNULL(index)
    GS_REQUIRE_NONNULL(key)
    DELEGATE_CALL_WARGS(index, _remove_interval, key);
    index->bounds.begin = (key->begin == index->bounds.begin)? DELEGATE_CALL(index, _minbegin) : index->bounds.begin;
    index->bounds.end = (key->end == index->bounds.end)? DELEGATE_CALL(index, _maxend) : index->bounds.end;
}

void gs_hindex_remove_having(struct gs_hindex_t *index, gs_tuple_id_t tid)
{
    DELEGATE_CALL_WARGS(index, _remove_intersec, tid);
}

bool gs_hindex_contains(const struct gs_hindex_t *index, gs_tuple_id_t tid)
{
    return DELEGATE_CALL_WARGS(index, _contains, tid);
}

gs_grid_cursor_t *gs_hindex_query(const struct gs_hindex_t *index, const gs_tuple_id_t *tid_begin,
                                  const gs_tuple_id_t *tid_end)
{
    GS_REQUIRE_NONNULL(tid_begin);
    GS_REQUIRE_NONNULL(tid_end);
    REQUIRE(tid_begin < tid_end, "Corrupted range");
    size_t approx_result_capacity = ((tid_end - tid_begin) * gs_schema_num_attributes(index->table_schema));
    gs_grid_cursor_t *result = gs_grid_cursor_new(approx_result_capacity);
    index->_query(result, index, tid_begin, tid_end);
    return result;
}

const struct gs_grid_t *gs_hindex_read(gs_grid_cursor_t *result_set)
{
    return gs_grid_cursor_next(result_set);
}

void gs_hindex_close(gs_grid_cursor_t *result_set)
{
    gs_grid_cursor_delete(result_set);
}

void gs_hindex_bounds(gs_tuple_id_interval_t *bounds, const gs_hindex_t *index)
{
    GS_REQUIRE_NONNULL(bounds);
    GS_REQUIRE_NONNULL(index);
    bounds->begin = index->bounds.begin;
    bounds->end = index->bounds.end;
}

void gs_hindex_print(FILE *file, const gs_hindex_t *index)
{
    gs_schema_t *print_schema = gs_schema_new("ad hoc info");
    attr_create_tupleid("begin", print_schema);
    attr_create_tupleid("end", print_schema);
    attr_create_gridid("grid id", print_schema);
    gs_frag_t *frag = gs_frag_new(print_schema, 1, FIT_HOST_NSM_FAT_VM);
    size_t dist = index->bounds.end - index->bounds.begin;
    gs_tuple_id_t *ids = GS_REQUIRE_MALLOC(dist * sizeof(gs_tuple_id_t));
    for (gs_tuple_id_t i = index->bounds.begin; i < index->bounds.end; i++) {
        ids[i] = i;
    }
    gs_grid_cursor_t *cursor = gs_hindex_query(index, ids, ids + dist);
    gs_tuplet_field_t field;
    gs_tuplet_t tuplet;

    /* dedup entries by hand */
    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);

    apr_hash_t *hash = apr_hash_make(pool);
    bool *dummy = GS_REQUIRE_MALLOC(sizeof(bool));
    *dummy = true;

    for (struct gs_grid_t *grid = gs_grid_cursor_next(cursor); grid != NULL; grid = gs_grid_cursor_next(NULL)) {
       size_t interval_idx = 0;
       const gs_tuple_id_interval_t *interval = gs_vec_at(grid->tuple_ids, interval_idx);
       print_tuple_t t = { .grid = grid->grid_id, .begin = interval->begin, .end = interval->end };
       if (apr_hash_get(hash, &t, sizeof(print_tuple_t)) == NULL) {
           gs_frag_insert(&tuplet, frag, 1);
           gs_tuplet_field_open(&field, &tuplet);
           gs_tuplet_field_write(&field, &interval->begin, true);
           gs_tuplet_field_write(&field, &interval->end, true);
           gs_tuplet_field_write(&field, &grid->grid_id, true);
           print_tuple_t *t_imp = apr_pmemdup(pool, &t, sizeof(print_tuple_t));
           apr_hash_set(hash, t_imp, sizeof(print_tuple_t), dummy);
       }
    }

    free(ids);
    apr_pool_destroy(pool);
    gs_frag_print(file, frag, 0, INT_MAX);
    gs_grid_cursor_delete(cursor);
    gs_frag_delete(frag);
    gs_schema_delete(print_schema);
}