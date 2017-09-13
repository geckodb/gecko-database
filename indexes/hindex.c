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

void gs_hindex_free(struct hindex_t *index)
{
    DELEGATE_CALL(index, _free);
}

void gs_hindex_add(struct hindex_t *index, const tuple_id_interval_t *key, const struct grid_t *grid)
{
    REQUIRE_NONNULL(index)
    REQUIRE_NONNULL(key)
    index->bounds.begin = min(index->bounds.begin, key->begin);
    index->bounds.end   = max(index->bounds.end, key->end);
    DELEGATE_CALL_WARGS(index, _add, key, grid);
}

void gs_hindex_remove_interval(struct hindex_t *index, const tuple_id_interval_t *key)
{
    REQUIRE_NONNULL(index)
    REQUIRE_NONNULL(key)
    DELEGATE_CALL_WARGS(index, _remove_interval, key);
    index->bounds.begin = (key->begin == index->bounds.begin)? DELEGATE_CALL(index, _minbegin) : index->bounds.begin;
    index->bounds.end = (key->end == index->bounds.end)? DELEGATE_CALL(index, _maxend) : index->bounds.end;
}

void gs_hindex_remove_intersec(struct hindex_t *index, tuple_id_t tid)
{
    DELEGATE_CALL_WARGS(index, _remove_intersec, tid);
}

bool gs_hindex_contains(const struct hindex_t *index, tuple_id_t tid)
{
    return DELEGATE_CALL_WARGS(index, _contains, tid);
}

grid_cursor_t *gs_hindex_query(const struct hindex_t *index, const tuple_id_t *tid_begin,
                                   const tuple_id_t *tid_end)
{
    REQUIRE_NONNULL(tid_begin);
    REQUIRE_NONNULL(tid_end);
    REQUIRE(tid_begin < tid_end, "Corrupted range");
    size_t approx_result_capacity = ((tid_end - tid_begin) * gs_schema_num_attributes(index->table_schema));
    grid_cursor_t *result = grid_cursor_create(approx_result_capacity);
    index->_query(result, index, tid_begin, tid_end);
    return result;
}

const struct grid_t *gs_hindex_query_read(grid_cursor_t *result_set)
{
    return grid_cursor_next(result_set);
}

void gs_hindex_query_close(grid_cursor_t *result_set)
{
    grid_cursor_close(result_set);
}

void gs_hindex_get_bounds(tuple_id_interval_t *bounds, const hindex_t *index)
{
    REQUIRE_NONNULL(bounds);
    REQUIRE_NONNULL(index);
    bounds->begin = index->bounds.begin;
    bounds->end = index->bounds.end;
}

void gs_hindex_print(FILE *file, const hindex_t *index)
{
    schema_t *print_schema = gs_schema_create("ad hoc info");
    gs_attr_create_tupleid("begin", print_schema);
    gs_attr_create_tupleid("end", print_schema);
    gs_attr_create_gridid("grid id", print_schema);
    frag_t *frag = gs_fragment_alloc(print_schema, 1, FIT_HOST_NSM_VM);
    size_t dist = index->bounds.end - index->bounds.begin;
    tuple_id_t *ids = REQUIRE_MALLOC(dist * sizeof(tuple_id_t));
    for (tuple_id_t i = index->bounds.begin; i < index->bounds.end; i++) {
        ids[i] = i;
    }
    grid_cursor_t *cursor = gs_hindex_query(index, ids, ids + dist);
    tuplet_t tuplet;

    /* dedup entries by hand */
    dict_t *dict = hash_table_create_jenkins(sizeof(print_tuple_t),
                                             sizeof(bool), 1.5f * grid_cursor_numelem(cursor), 1.7f, 0.75f);

    for (struct grid_t *grid = grid_cursor_next(cursor); grid != NULL; grid = grid_cursor_next(NULL)) {
       size_t interval_idx = 0;
       const tuple_id_interval_t *interval = vector_at(grid->tuple_ids, interval_idx); bool dummy;
       print_tuple_t t = { .grid = grid->grid_id, .begin = interval->begin, .end = interval->end };
       if (!dict_contains_key(dict, &t)) {
           gs_frag_insert(&tuplet, frag, 1);
           gs_tuplet_field_open(&tuplet);
           tuplet_field_t *field = gs_tuplet_field_open(&tuplet);
           gs_tuplet_field_write(field, &interval->begin, true);
           gs_tuplet_field_write(field, &interval->end, true);
           gs_tuplet_field_write(field, &grid->grid_id, true);
           gs_tuplet_field_close(field);
           dict_put(dict, &t, &dummy);
       }
    }

    free(ids);
    dict_free(dict);
    gs_frag_print(file, frag, 0, INT_MAX);
    grid_cursor_close(cursor);
    gs_frag_free(frag);
    gs_schema_free(print_schema);
}