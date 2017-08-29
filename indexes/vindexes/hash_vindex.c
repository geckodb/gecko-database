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

#include <indexes/vindexes/hash_vindex.h>
#include <containers/dicts/hash_table.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define require_besearch_hindex_tag(index)                                                                             \
    require((index->tag == GI_VINDEX_HASH), BADTAG);

#define require_instanceof_this(index)                                                                                 \
    { require_nonnull(index); require_nonnull(index->extra); require_besearch_hindex_tag(index); }

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline void this_add(struct vindex_t *self, const void *key, const struct grid_t *grid);
static inline void this_remove(struct vindex_t *self, const void *key);
static inline bool this_contains(const struct vindex_t *self, const void *key);
static inline void this_free(struct vindex_t *self);
static inline void this_query(grid_index_result_cursor_t *result, const struct vindex_t *self, const void *key_begin,
                              const void *key_end);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

vindex_t *hash_vindex_create(size_t key_size, size_t num_init_slots,
                             bool (*equals)(const void *key_lhs, const void *key_rhs),
                             void (*cleanup)(void *key, void *value))
{
    vindex_t *result = require_good_malloc(sizeof(vindex_t));
    *result = (vindex_t) {
        ._add = this_add,
        ._contains = this_contains,
        ._free = this_free,
        ._query = this_query,
        ._remove = this_remove,
        .tag = GI_VINDEX_HASH,
    };

    result->extra = hash_table_create_ex(
            &(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen}, key_size, sizeof(vector_t),
            num_init_slots, num_init_slots, 1.7f, 0.75f, equals, cleanup, false
    );
    require_non_null(result->extra);
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline void this_add(struct vindex_t *self, const void *key, const struct grid_t *grid)
{
    require_instanceof_this(self);
    dict_t *dict = ((dict_t *)self->extra);
    if (!dict_contains_key(dict, key)) {
        vector_t *vec = vector_create(sizeof(struct grid_t *), 10);
        dict_put(dict, key, vec);
        // Notice, this frees up pointer to vec, but does not cleanup the vector (especially the data pointer).
        // That's required since dict_put copies all members of vec and is responsible to free up resources.
        // The allocated memory for the pointer to original vector "vec", however, must be freed also.
        free (vec);
    }
    vector_t *vec = (vector_t *) dict_get(dict, key);
    require_non_null(vec);
    vector_add(vec, 1, &grid);
}

static inline void this_remove(struct vindex_t *self, const void *key)
{
    require_instanceof_this(self);
    panic(NOTIMPLEMENTED, to_string(this_remove)) // requires proper implementation of remove in hash table
}

static inline bool this_contains(const struct vindex_t *self, const void *key)
{
    require_instanceof_this(self);
    dict_t *dict = ((dict_t *)self->extra);
    require_non_null(dict);
    return dict_contains_key(dict, key);
}

void this_free(struct vindex_t *self)
{
    require_instanceof_this(self);
    dict_free((dict_t *) self->extra);
}


static inline void this_query(grid_index_result_cursor_t *result, const struct vindex_t *self, const void *key_begin,
                              const void *key_end)
{
    require_non_null(result->extra);
    for (const void *key = key_begin; key != key_end; key++) {
        if (this_contains(self, key)) {
            dict_t *dict = ((dict_t *)self->extra);
            const void *grid = dict_get(dict, key);
            vector_add((vector_t *) result->extra, 1, &grid);
        }
    }
}

