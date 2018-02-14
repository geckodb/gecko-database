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

#include <indexes/gs_vindex.h>
#include <gs_grid.h>
#include <gs_tuplet_field.h>
#include <gs_tuplet_field.h>

void gs_vindex_delete(gs_vindex_t *index)
{
    DELEGATE_CALL(index, _free);
}

void gs_vindex_add(gs_vindex_t *index, const gs_attr_id_t *key, const struct gs_grid_t *grid)
{
    DELEGATE_CALL_WARGS(index, _add, key, grid);
    hashset_add(&index->keys, key, 1, GS_ATTR_ID_COMP);
}

void gs_vindex_remove(gs_vindex_t *index, const gs_attr_id_t *key)
{
    DELEGATE_CALL_WARGS(index, _remove, key);
    hashset_remove(&index->keys, key, 1);
}

bool gs_vindex_contains(const gs_vindex_t *index, const gs_attr_id_t *key)
{
    return DELEGATE_CALL_WARGS(index, _contains, key);
}

gs_grid_cursor_t *gs_vindex_query(const struct gs_vindex_t *index, const gs_attr_id_t *key_range_begin,
                            const gs_attr_id_t *key_range_end)
{
    GS_REQUIRE_NONNULL(key_range_begin);
    GS_REQUIRE_NONNULL(key_range_end);
    REQUIRE(key_range_begin < key_range_end, "Corrupted range");
    size_t result_capacity = (key_range_end - key_range_begin);
    gs_grid_cursor_t *result = gs_grid_cursor_new(result_capacity);
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

gs_grid_cursor_t *gs_vindex_query_append(const struct gs_vindex_t *index, gs_grid_cursor_t *result,
                                   const gs_attr_id_t *key_range_begin, const gs_attr_id_t *key_range_end)
{
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

const struct gs_grid_t *gs_vindex_read(gs_grid_cursor_t *result_set)
{
    return gs_grid_cursor_next(result_set);
}

void gs_vindex_close(gs_grid_cursor_t *result_set)
{
    gs_grid_cursor_delete(result_set);
}

const gs_attr_id_t *gs_vindex_begin(const gs_vindex_t *index)
{
    GS_REQUIRE_NONNULL(index);
    return hashset_begin(&index->keys);
}

const gs_attr_id_t *gs_vindex_end(const gs_vindex_t *index)
{
    GS_REQUIRE_NONNULL(index);
    return hashset_end(&index->keys);
}

void gs_vindex_print(FILE *file, gs_vindex_t *index)
{
    GS_REQUIRE_NONNULL(file);
    GS_REQUIRE_NONNULL(index);

    gs_schema_t *print_schema = gs_schema_new("ad hoc info");
    attr_create_strptr("attribute", print_schema);
    attr_create_gridid("grid id", print_schema);
    gs_frag_t *frag = gs_frag_new(print_schema, 1, FIT_HOST_NSM_FAT_VM);
    gs_tuplet_t tuplet;
    gs_tuplet_field_t field;

    for (const gs_attr_id_t *it = gs_vindex_begin(index); it < gs_vindex_end(index); it++) {
        gs_grid_cursor_t *cursor = gs_vindex_query(index, it, it + 1);
        for (struct gs_grid_t *grid = gs_grid_cursor_next(cursor); grid != NULL; grid = gs_grid_cursor_next(NULL)) {
            gs_frag_insert(&tuplet, frag, 1);
            gs_tuplet_field_open(&field, &tuplet);
            gs_tuplet_field_write(&field, gs_table_attr_by_id(grid->context, *it)->name, true);
            gs_tuplet_field_write(&field, &grid->grid_id, true);
        }
        gs_grid_cursor_delete(cursor);
    }

    gs_frag_print(file, frag, 0, INT_MAX);
    gs_frag_delete(frag);
    gs_schema_delete(print_schema);

}
