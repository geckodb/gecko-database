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

#include <indexes/vindex.h>
#include <grid.h>
#include <tuplet_field.h>

void vindex_delete(vindex_t *index)
{
    DELEGATE_CALL(index, _free);
}

void vindex_add(vindex_t *index, const attr_id_t *key, const struct grid_t *grid)
{
    DELEGATE_CALL_WARGS(index, _add, key, grid);
    hashset_add(&index->keys, key, 1);
}

void vindex_remove(vindex_t *index, const attr_id_t *key)
{
    DELEGATE_CALL_WARGS(index, _remove, key);
    hashset_remove(&index->keys, key, 1);
}

bool vindex_contains(const vindex_t *index, const attr_id_t *key)
{
    return DELEGATE_CALL_WARGS(index, _contains, key);
}

grid_cursor_t *vindex_query(const struct vindex_t *index, const attr_id_t *key_range_begin,
                            const attr_id_t *key_range_end)
{
    REQUIRE_NONNULL(key_range_begin);
    REQUIRE_NONNULL(key_range_end);
    REQUIRE(key_range_begin < key_range_end, "Corrupted range");
    size_t result_capacity = (key_range_end - key_range_begin);
    grid_cursor_t *result = grid_cursor_new(result_capacity);
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

grid_cursor_t *vindex_query_append(const struct vindex_t *index, grid_cursor_t *result,
                                   const attr_id_t *key_range_begin, const attr_id_t *key_range_end)
{
    index->_query(result, index, key_range_begin, key_range_end);
    return result;
}

const struct grid_t *vindex_read(grid_cursor_t *result_set)
{
    return grid_cursor_next(result_set);
}

void vindex_close(grid_cursor_t *result_set)
{
    grid_cursor_delete(result_set);
}

const attr_id_t *vindex_begin(const vindex_t *index)
{
    REQUIRE_NONNULL(index);
    return hashset_begin(&index->keys);
}

const attr_id_t *vindex_end(const vindex_t *index)
{
    REQUIRE_NONNULL(index);
    return hashset_end(&index->keys);
}

void vindex_print(FILE *file, vindex_t *index)
{
    REQUIRE_NONNULL(file);
    REQUIRE_NONNULL(index);

    schema_t *print_schema = schema_new("ad hoc info");
    attr_create_strptr("attribute", print_schema);
    attr_create_gridid("grid id", print_schema);
    frag_t *frag = frag_new(print_schema, 1, FIT_HOST_NSM_VM);
    tuplet_t tuplet;
    tuplet_field_t field;

    for (const attr_id_t *it = vindex_begin(index); it < vindex_end(index); it++) {
        grid_cursor_t *cursor = vindex_query(index, it, it + 1);
        for (struct grid_t *grid = grid_cursor_next(cursor); grid != NULL; grid = grid_cursor_next(NULL)) {
            frag_insert(&tuplet, frag, 1);
            tuplet_field_open(&field, &tuplet);
            tuplet_field_write(&field, table_attr_by_id(grid->context, *it)->name, true);
            tuplet_field_write(&field, &grid->grid_id, true);
        }
        grid_cursor_delete(cursor);
    }

    frag_print(file, frag, 0, INT_MAX);
    frag_delete(frag);
    schema_delete(print_schema);

}
