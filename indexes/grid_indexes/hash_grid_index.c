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

#include <indexes/grid_indexes/hash_grid_index.h>
#include <containers/dicts/hash_table.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define require_hash_grid_index_tag(index)                                                                             \
    require((index->tag == GIT_HASH), BADTAG);

#define require_instanceof_this(index)                                                                                 \
    { require_nonnull(index); require_nonnull(index->extra); require_hash_grid_index_tag(index); }

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline void insert(index_query_t *result, const struct grid_index_t *self, size_t key_begin, size_t key_end);

static inline void this_add(struct grid_index_t *self, size_t key, const struct grid_t *grid);
static inline void this_remove(struct grid_index_t *self, size_t key);
static inline bool this_contains(const struct grid_index_t *self, size_t key);
static inline index_query_t *this_open(const struct grid_index_t *self, size_t key_begin, size_t key_end);
static inline index_query_t *this_append(const struct grid_index_t *self, index_query_t *result, size_t key_begin,
                                         size_t key_end);
static inline const struct grid_t *this_read(const struct grid_index_t *self, index_query_t *result_set);
static inline void this_close(index_query_t *result_set);
static inline void this_free(struct grid_index_t *self);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

grid_index_t *hash_grid_index_create(size_t num_init_slots)
{
    grid_index_t *result = require_good_malloc(sizeof(grid_index_t));
    *result = (grid_index_t) {
        ._add = this_add,
        ._contains = this_contains,
        ._free = this_free,
        ._query_open = this_open,
        ._query_append = this_append,
        ._query_read = this_read,
        ._query_close = this_close,
        ._remove = this_remove,
        .tag = GIT_HASH,
        .extra = require_good_malloc(sizeof(dict_t))
    };

    result->extra = hash_table_create(
            &(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen}, sizeof(size_t), sizeof(vector_t *),
            num_init_slots, 1.7f, 0.75f
    );
    require_non_null(result->extra);
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline void insert(index_query_t *result, const struct grid_index_t *self, size_t key_begin, size_t key_end)
{
    require_non_null(result->extra);
    for (size_t key = key_begin; key < key_end; key++) {
        if (this_contains(self, key)) {
            dict_t *dict = ((dict_t *)self->extra);
            const void *grid = dict_get(dict, &key);
            vector_add((vector_t *) result->extra, 1, &grid);
        }
    }
}

static inline void this_add(struct grid_index_t *self, size_t key, const struct grid_t *grid)
{
    require_instanceof_this(self);
    dict_t *dict = ((dict_t *)self->extra);
    if (!dict_contains_key(dict, &key)) {
        vector_t *vec = vector_create(sizeof(struct grid_t *), 10);
        dict_put(dict, &key, &vec);
    }
    vector_t *vec = (vector_t *) dict_get(dict, &key);
    require_non_null(vec);
    vector_add(vec, 1, &grid);
}

static inline void this_remove(struct grid_index_t *self, size_t key)
{
    require_instanceof_this(self);
    panic(NOTIMPLEMENTED, to_string(this_remove)) // requires proper implementation of remove in hash table
}

static inline bool this_contains(const struct grid_index_t *self, size_t key)
{
    require_instanceof_this(self);
    dict_t *dict = ((dict_t *)self->extra);
    require_non_null(dict);
    return dict_contains_key(dict, &key);
}

static inline index_query_t *this_open(const struct grid_index_t *self, size_t key_begin, size_t key_end)
{
    require_instanceof_this(self);
    require((key_begin < key_end), BADRANGEBOUNDS);
    size_t result_capacity = (key_end - key_begin);
    index_query_t *result = require_good_malloc(sizeof(index_query_t));
    *result = (index_query_t) {
        .tag = GIT_HASH,
        .extra = vector_create(sizeof(struct grid_t *), result_capacity)
    };
    insert(result, self, key_begin, key_end);
    return result;
}

static inline index_query_t *this_append(const struct grid_index_t *self, index_query_t *result, size_t key_begin,
                                         size_t key_end)
{
    require_instanceof_this(self);
    require_instanceof_this(result);
    require((key_begin < key_end), BADRANGEBOUNDS);
    insert(result, self, key_begin, key_end);
    return result;
}

static inline const struct grid_t *this_read(const struct grid_index_t *self, index_query_t *result_set)
{
    require_instanceof_this(self);
    require_instanceof_this(result_set);

    static index_query_t *dest;
    static size_t elem_idx;
    if (result_set != NULL) {
        dest = result_set;
        elem_idx = 0;
    }

    vector_t *vec = (vector_t * ) dest->extra;
    if (elem_idx < vec->sizeof_element) {
        return (const struct grid_t *) vector_at(vec, elem_idx++);
    } else return NULL;

}

void this_close(index_query_t *result_set)
{
    require_instanceof_this(result_set);
    vector_free((vector_t * ) result_set->extra);
}

void this_free(struct grid_index_t *self)
{
    require_instanceof_this(self);
    dict_free((dict_t *) self->extra);
}