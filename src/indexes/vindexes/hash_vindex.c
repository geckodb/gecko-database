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

#include <indexes/vindexes/hash_vindex.h>
#include <containers/dicts/hash_table.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define require_besearch_hindex_tag(index)                                                                             \
    REQUIRE((index->tag == GI_VINDEX_HASH), BADTAG);

#define REQUIRE_INSTANCEOF_THIS(index)                                                                                 \
    { REQUIRE_NONNULL(index); REQUIRE_NONNULL(index->extra); require_besearch_hindex_tag(index); }

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline void this_add(struct vindex_t *self, const attr_id_t *key, const struct grid_t *grid);
static inline void this_remove(struct vindex_t *self, const attr_id_t *key);
static inline bool this_contains(const struct vindex_t *self, const attr_id_t *key);
static inline void this_free(struct vindex_t *self);
static inline void this_query(grid_cursor_t *result, const struct vindex_t *self, const attr_id_t *key_begin,
                              const attr_id_t *key_end);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline bool attr_key_equals(const void *key_lhs, const void *key_rhs)
{
    attr_id_t lhs = *((attr_id_t *) key_lhs);
    attr_id_t rhs = *((attr_id_t *) key_rhs);
    return (lhs == rhs);
}

static inline void cleanup_vectors(void *key, void *value)
{
    vec_dispose((vec_t *) value);
}

vindex_t *hash_vindex_new(size_t key_size, size_t num_init_slots)
{
    vindex_t *result = REQUIRE_MALLOC(sizeof(vindex_t));
    *result = (vindex_t) {
        ._add = this_add,
        ._contains = this_contains,
        ._free = this_free,
        ._query = this_query,
        ._remove = this_remove,
        .tag = GI_VINDEX_HASH
    };

    hashset_create(&result->keys, key_size, num_init_slots);

    result->extra = hash_table_new_ex(
            &(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen}, key_size, sizeof(vec_t),
            num_init_slots, num_init_slots, 1.7f, 0.75f, attr_key_equals, cleanup_vectors, false
    );
    REQUIRE_NONNULL(result->extra);
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline void this_add(struct vindex_t *self, const attr_id_t *key, const struct grid_t *grid)
{
    REQUIRE_INSTANCEOF_THIS(self);
    dict_t *dict = ((dict_t *)self->extra);
    if (!dict_contains_key(dict, key)) {
        vec_t *vec = vec_new(sizeof(struct grid_t *), 10);
        dict_put(dict, key, vec);
        // Notice, this frees up pointer to vec, but does not cleanup the vector (especially the data pointer).
        // That's required since dict_put copies all members of vec and is responsible to free up resources.
        // The allocated memory for the pointer to original vector "vec", however, must be freed also.
        free (vec);
    }
    vec_t *vec = (vec_t *) dict_get(dict, key);
    REQUIRE_NONNULL(vec);
    vec_pushback(vec, 1, &grid);
}

static inline void this_remove(struct vindex_t *self, const attr_id_t *key)
{
    REQUIRE_INSTANCEOF_THIS(self);
    panic(NOTIMPLEMENTED, to_string(this_remove)) // requires proper implementation of remove in hash table
}

static inline bool this_contains(const struct vindex_t *self, const attr_id_t *key)
{
    REQUIRE_INSTANCEOF_THIS(self);
    dict_t *dict = ((dict_t *)self->extra);
    REQUIRE_NONNULL(dict);
    return dict_contains_key(dict, key);
}

void this_free(struct vindex_t *self)
{
    REQUIRE_INSTANCEOF_THIS(self);
    dict_delete((dict_t *) self->extra);
    hashset_dispose(&self->keys);
}


static inline void this_query(grid_cursor_t *result, const struct vindex_t *self, const attr_id_t *key_begin,
                              const attr_id_t *key_end)
{
    REQUIRE_NONNULL(result->extra);
    for (const attr_id_t *key = key_begin; key != key_end; key++) {
        if (this_contains(self, key)) {
            dict_t *dict = ((dict_t *)self->extra);
            const struct vec_t *vec = dict_get(dict, key);
            vec_add_all(result->extra, vec);
        }
    }
}

