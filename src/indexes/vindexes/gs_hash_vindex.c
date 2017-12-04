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

#include <apr_strings.h>
#include <apr_hash.h>

#include <indexes/vindexes/gs_hash_vindex.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define require_besearch_hindex_tag(index)                                                                             \
    REQUIRE((index->tag == GI_VINDEX_HASH), BADTAG);

#define REQUIRE_INSTANCEOF_THIS(index)                                                                                 \
    { GS_REQUIRE_NONNULL(index); GS_REQUIRE_NONNULL(index->extra); require_besearch_hindex_tag(index); }

typedef struct hash_vindex_extra_t {
    apr_pool_t *pool;
    apr_hash_t *hash;
    size_t key_size;
} hash_vindex_extra_t;

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static void this_add(struct gs_vindex_t *self, const gs_attr_id_t *key, const struct gs_grid_t *grid);
static void this_remove(struct gs_vindex_t *self, const gs_attr_id_t *key);
static bool this_contains(const struct gs_vindex_t *self, const gs_attr_id_t *key);
static void this_free(struct gs_vindex_t *self);
static void this_query(gs_grid_cursor_t *result, const struct gs_vindex_t *self, const gs_attr_id_t *key_begin,
                              const gs_attr_id_t *key_end);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

 bool attr_key_equals(const void *key_lhs, const void *key_rhs)
{
    gs_attr_id_t lhs = *((gs_attr_id_t *) key_lhs);
    gs_attr_id_t rhs = *((gs_attr_id_t *) key_rhs);
    return (lhs == rhs);
}

 void cleanup_vectors(void *key, void *value)
{
    gs_vec_dispose((gs_vec_t *) value);
}

gs_vindex_t *gs_hash_vindex_new(size_t num_init_slots)   // TODO: remove key_size; it's gs_attr_id_t always!
{
    gs_vindex_t *result = GS_REQUIRE_MALLOC(sizeof(gs_vindex_t));
    *result = (gs_vindex_t) {
        ._add = this_add,
        ._contains = this_contains,
        ._free = this_free,
        ._query = this_query,
        ._remove = this_remove,
        .tag = GI_VINDEX_HASH
    };

    hashset_create(&result->keys, sizeof(gs_attr_id_t), num_init_slots, GS_ATTR_ID_COMP);

    hash_vindex_extra_t *extra = GS_REQUIRE_MALLOC(sizeof(hash_vindex_extra_t));
    apr_pool_create(&extra->pool, NULL);
    extra->hash = apr_hash_make(extra->pool);
    extra->key_size = sizeof(gs_attr_id_t);

    result->extra = extra;

    GS_REQUIRE_NONNULL(result->extra);
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static void this_add(struct gs_vindex_t *self, const gs_attr_id_t *key, const struct gs_grid_t *grid)
{
    REQUIRE_INSTANCEOF_THIS(self);
    hash_vindex_extra_t *extra = ((hash_vindex_extra_t *)self->extra);
    if (!apr_hash_get(extra->hash, key, extra->key_size)) {
        gs_vec_t *vec = gs_vec_new(sizeof(struct grid_t *), 10);
        gs_attr_id_t *key_imp = apr_pmemdup(extra->pool, key, sizeof(gs_attr_id_t));
        apr_hash_set(extra->hash, key_imp, sizeof(gs_attr_id_t), vec);
    }
    gs_vec_t *vec = (gs_vec_t *) apr_hash_get(extra->hash, key, sizeof(gs_attr_id_t));
    GS_REQUIRE_NONNULL(vec);
    gs_vec_pushback(vec, 1, &grid);
}

static void this_remove(struct gs_vindex_t *self, const gs_attr_id_t *key)
{
    REQUIRE_INSTANCEOF_THIS(self);
    panic(NOTIMPLEMENTED, to_string(this_remove)) // requires proper implementation of remove in hash table
}

static bool this_contains(const struct gs_vindex_t *self, const gs_attr_id_t *key)
{
    REQUIRE_INSTANCEOF_THIS(self);
    hash_vindex_extra_t *extra = ((hash_vindex_extra_t *)self->extra);
    GS_REQUIRE_NONNULL(extra);
    GS_REQUIRE_NONNULL(extra->hash);
    return (apr_hash_get(extra->hash, key, sizeof(gs_attr_id_t)) != NULL);
}

static void this_free(struct gs_vindex_t *self)
{
    REQUIRE_INSTANCEOF_THIS(self);
    hash_vindex_extra_t *extra = ((hash_vindex_extra_t *)self->extra);
    apr_pool_destroy(extra->pool);
    hashset_dispose(&self->keys);
    free (extra);
}


static void this_query(gs_grid_cursor_t *result, const struct gs_vindex_t *self, const gs_attr_id_t *key_begin,
                              const gs_attr_id_t *key_end)
{
    GS_REQUIRE_NONNULL(result->extra);
    for (const gs_attr_id_t *key = key_begin; key != key_end; key++) {
        if (this_contains(self, key)) {
            hash_vindex_extra_t *extra = ((hash_vindex_extra_t *)self->extra);
            const struct gs_vec_t *vec = apr_hash_get(extra->hash, key, sizeof(gs_attr_id_t));
            gs_vec_add_all(result->extra, vec);
        }
    }
}
