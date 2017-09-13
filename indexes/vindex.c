#include <indexes/vindex.h>
#include <grid.h>
#include <tuplet_field.h>

void gs_vindex_free(vindex_t *index)
{
    DELEGATE_CALL(index, _free);
}

void gs_vindex_add(vindex_t *index, const attr_id_t *key, const struct grid_t *grid)
{
    DELEGATE_CALL_WARGS(index, _add, key, grid);
    gs_hashset_add(&index->keys, key, 1);
}

void gs_vindex_remove(vindex_t *index, const attr_id_t *key)
{
    DELEGATE_CALL_WARGS(index, _remove, key);
    gs_hashset_remove(&index->keys, key, 1);
}

bool gs_vindex_contains(const vindex_t *index, const attr_id_t *key)
{
    return DELEGATE_CALL_WARGS(index, _contains, key);
}

grid_cursor_t *gs_vindex_query(const struct vindex_t *index, const attr_id_t *key_range_begin,
                                   const attr_id_t *key_range_end)
{
    REQUIRE_NONNULL(key_range_begin);
    REQUIRE_NONNULL(key_range_end);
    REQUIRE(key_range_begin < key_range_end, "Corrupted range");
    size_t result_capacity = (key_range_end - key_range_begin);
    grid_cursor_t *result = grid_cursor_create(result_capacity);
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

grid_cursor_t *gs_vindex_query_append(const struct vindex_t *index, grid_cursor_t *result,
                                        const attr_id_t *key_range_begin, const attr_id_t *key_range_end)
{
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

const struct grid_t *gs_vindex_query_read(grid_cursor_t *result_set)
{
    return grid_cursor_next(result_set);
}

void gs_vindex_query_close(grid_cursor_t *result_set)
{
    grid_cursor_close(result_set);
}

const attr_id_t *gs_vindex_keys_begin(const vindex_t *index)
{
    REQUIRE_NONNULL(index);
    return gs_hashset_begin(&index->keys);
}

const attr_id_t *gs_vindex_keys_end(const vindex_t *index)
{
    REQUIRE_NONNULL(index);
    return gs_hashset_end(&index->keys);
}

void gs_vindex_print(FILE *file, vindex_t *index)
{
    REQUIRE_NONNULL(file);
    REQUIRE_NONNULL(index);

    schema_t *print_schema = gs_schema_create("ad hoc info");
    gs_attr_create_strptr("attribute", print_schema);
    gs_attr_create_gridid("grid id", print_schema);
    frag_t *frag = gs_fragment_alloc(print_schema, 1, FIT_HOST_NSM_VM);
    tuplet_t tuplet;

    for (const attr_id_t *it = gs_vindex_keys_begin(index); it < gs_vindex_keys_end(index); it++) {
        grid_cursor_t *cursor = gs_vindex_query(index, it, it + 1);
        for (struct grid_t *grid = grid_cursor_next(cursor); grid != NULL; grid = grid_cursor_next(NULL)) {
            gs_frag_insert(&tuplet, frag, 1);
            gs_tuplet_field_open(&tuplet);
            tuplet_field_t *field = gs_tuplet_field_open(&tuplet);
            gs_tuplet_field_write(field, gs_grid_table_attr_by_id(grid->context, *it)->name, true);
            gs_tuplet_field_write(field, &grid->grid_id, true);
            gs_tuplet_field_close(field);
        }
        grid_cursor_close(cursor);
    }

    gs_frag_print(file, frag, 0, INT_MAX);
    gs_frag_free(frag);
    gs_schema_free(print_schema);

}
