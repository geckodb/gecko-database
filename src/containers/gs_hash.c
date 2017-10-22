// An implementation of the vector data structure
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

#include <containers/gs_hash.h>
#include <containers/gs_freelist.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define GET_HASH_CODE(key, key_size)                                \
    (hash->hash_func(NULL, key_size, key) % hash->num_buckets)


// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_hash_func_t {
    const void         *capture;
    gs_hash_code_fn_t      hash_code;
} gs_hash_func_t;

typedef struct hash_bucket_entry_t {
    bool                 in_use;
    gs_comp_func_t       key_comp;
    const void          *key;
    const void          *value;
} hash_bucket_entry_t;

typedef struct hash_bucket_t {
    gs_vec_t               *entries;
    gs_freelist_t          *freelist;
} hash_bucket_t;

typedef struct gs_hash_t {
    hash_bucket_t       *buckets;
    size_t               num_buckets;
    gs_hash_code_fn_t       hash_func;
} gs_hash_t;

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline void setup_hash_buckets(hash_bucket_t *buckets, size_t num_buckets, gs_comp_func_t key_comp);
static inline void cleanup_hash_buckets(hash_bucket_t *buckets, size_t num_buckets);
static inline void hash_bucket_init(hash_bucket_t *bucket, gs_comp_func_t key_comp);
static inline void hash_bucket_set(hash_bucket_t *bucket, void *key, void *value, gs_comp_func_t key_comp);
static inline void hash_bucket_unset(hash_bucket_t *bucket, void *key);
static inline const void *hash_bucket_get(hash_bucket_t *bucket, void *key);
static inline void hash_bucket_cleanup(hash_bucket_t *bucket);
static inline void hash_bucket_entry_init(hash_bucket_entry_t *entry, gs_comp_func_t key_comp);
static inline int entry_comp_by_key(const void *a, const void *b);

static inline void size_t_init(void *data);
static inline void size_t_inc(void *data);



// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------


GS_DECLARE(gs_status_t) gs_hash_create(gs_hash_t **hash, size_t num_buckets, gs_comp_func_t key_comp)
{
    return gs_hash_create_ex(hash, num_buckets, gs_hash_code_jen, key_comp);
}

GS_DECLARE(gs_status_t) gs_hash_create_ex(gs_hash_t **hash, size_t num_buckets, gs_hash_code_fn_t hash_func, gs_comp_func_t key_comp)
{
    gs_hash_t *result   = GS_REQUIRE_MALLOC(sizeof(gs_hash_t));
    result->buckets     = GS_REQUIRE_MALLOC(sizeof(hash_bucket_t));
    result->hash_func   = hash_func;
    result->num_buckets = num_buckets;
    setup_hash_buckets(result->buckets, result->num_buckets, key_comp);
    *hash = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_hash_dispose(gs_hash_t *hash)
{
    GS_REQUIRE_NONNULL(hash);
    cleanup_hash_buckets(hash->buckets, hash->num_buckets);
    free(hash);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_hash_set(gs_hash_t *hash, void *key, size_t key_size, void *val, gs_comp_func_t key_comp)
{
    return gs_hash_set_ex(hash, key, key_size, val, 1, key_comp);
}

GS_DECLARE(gs_status_t) gs_hash_set_ex(gs_hash_t *hash, void *keys, size_t key_size, void *vals, size_t num_elems,
                                       gs_comp_func_t key_comp)
{
    GS_REQUIRE_NONNULL(hash);
    GS_REQUIRE_NONNULL(keys);
    GS_REQUIRE_NONNULL(vals);

    while (num_elems--) {
        void *key = keys++;
        void *val = vals++;
        size_t bucket_idx = GET_HASH_CODE(key, key_size);
        hash_bucket_set(hash->buckets + bucket_idx, key, val, key_comp);
    }

    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_hash_unset_ex(gs_hash_t *hash, void *keys, size_t key_size, size_t num_elems)
{
    GS_REQUIRE_NONNULL(hash);
    GS_REQUIRE_NONNULL(keys);

    while (num_elems--) {
        void *key = keys++;
        size_t bucket_idx = GET_HASH_CODE(key, key_size);
        hash_bucket_unset(hash->buckets + bucket_idx, key);
    }

    return GS_SUCCESS;
}

GS_DECLARE(void *) gs_hash_get(const gs_hash_t *hash, void *key, size_t key_size)
{
    gs_vec_t *result = gs_vec_new(sizeof(void *), 1);
    gs_hash_get_ex(result, hash, key, key_size, 1);
    void *retval = (gs_vec_length(result) != 0 ? *(void **) gs_vec_at(result, 0) : NULL);
    gs_vec_free(result);
    return retval;
}

GS_DECLARE(gs_status_t) gs_hash_get_ex(gs_vec_t *result, const gs_hash_t *hash, void *keys, size_t key_size,
                                       size_t num_elems)
{
    GS_REQUIRE_NONNULL(result);
    GS_REQUIRE_NONNULL(hash);
    GS_REQUIRE_NONNULL(keys);

    while (num_elems--) {
        void *key = keys++;
        size_t bucket_idx = GET_HASH_CODE(key, key_size);
        const void *value;
        if ((value = hash_bucket_get(hash->buckets + bucket_idx, key)) != NULL) {
            gs_vec_pushback(result, 1, &value);
        }
    }

    return GS_SUCCESS;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline void setup_hash_buckets(hash_bucket_t *buckets, size_t num_buckets, gs_comp_func_t key_comp)
{
    while (num_buckets--) {
        hash_bucket_init(buckets++, key_comp);
    }
}

static inline void cleanup_hash_buckets(hash_bucket_t *buckets, size_t num_buckets)
{
    while (num_buckets--) {
        hash_bucket_cleanup(buckets++);
    }
}

static inline void hash_bucket_init(hash_bucket_t *bucket, gs_comp_func_t key_comp)
{
    gs_freelist_create2(&bucket->freelist, sizeof(size_t), 10, size_t_init, size_t_inc);
    bucket->entries = gs_vec_new(sizeof(hash_bucket_entry_t), GS_HASH_BUCKET_CAP);

    for (size_t i = 0; i < GS_HASH_BUCKET_CAP; i++) {
        hash_bucket_entry_t entry;
        hash_bucket_entry_init(&entry, key_comp);
        gs_vec_pushback(bucket->entries, 1, &entry);
    }
    gs_vec_resize(bucket->entries, 0);
}

static inline void hash_bucket_set(hash_bucket_t *bucket, void *key, void *value, gs_comp_func_t key_comp)
{
    hash_bucket_entry_t new_entry = { .key = key, .value = value, .in_use = true, .key_comp = key_comp };
    hash_bucket_entry_t *it = gs_vec_bsearch(bucket->entries, &new_entry, entry_comp_by_key, entry_comp_by_key);
    if (it == gs_vec_end(bucket->entries)) {
        /* no entry with the given key */
        size_t entry_idx;
        gs_freelist_bind(&entry_idx, bucket->freelist, 1);
        gs_vec_set(bucket->entries, entry_idx, 1, &new_entry);
    } else {
        /* key was already bound to a value; update the entry */
        *it = new_entry;
    }
}

static inline void hash_bucket_unset(hash_bucket_t *bucket, void *key)
{
    hash_bucket_entry_t new_entry = { .key = key };
    hash_bucket_entry_t *it = gs_vec_bsearch(bucket->entries, &new_entry, entry_comp_by_key, entry_comp_by_key);
    if (it != gs_vec_end(bucket->entries)) {
        it->in_use = false;
        size_t idx = ((void *) it - gs_vec_begin(bucket->entries));
        gs_freelist_pushback(bucket->freelist, 1, &idx);
    }
}

static inline const void *hash_bucket_get(hash_bucket_t *bucket, void *key)
{
    hash_bucket_entry_t new_entry = { .key = key };
    hash_bucket_entry_t *it = gs_vec_bsearch(bucket->entries, &new_entry, entry_comp_by_key, entry_comp_by_key);
    if (it != gs_vec_end(bucket->entries) && it->in_use) {
        return it->value;
    } else return NULL;
}

static inline void hash_bucket_cleanup(hash_bucket_t *bucket)
{
    gs_freelist_free(bucket->freelist);
    free(bucket->entries);
}

static inline void hash_bucket_entry_init(hash_bucket_entry_t *entry, gs_comp_func_t key_comp)
{
    entry->in_use = false;
    entry->key    = NULL;
    entry->value  = NULL;

    /* TODO: Re-implement clib mergesort with a sort comp function in which a user-def comp function can be injected
     * Note that storing the key_comp pointer in each bucket entry is a horrible waste of memory but it is required for
     * a "fast" implementation of "entry_comp_by_key" for searching and finding with standard lib functions. In the
     * future, the clib search function can be replaced with a custom search (i.e., add capture to functions) */
    entry->key_comp = key_comp;
}

static inline int entry_comp_by_key(const void *a, const void *b)
{
    hash_bucket_entry_t *lhs = (hash_bucket_entry_t *) a;
    hash_bucket_entry_t *rhs = (hash_bucket_entry_t *) b;
    gs_comp_func_t comp = (lhs->key_comp != NULL ? lhs->key_comp : rhs->key_comp);
    return comp(lhs->key, rhs->key);
}

static inline void size_t_init(void *data) { *((size_t *) data) = 0; }

static inline void size_t_inc(void *data)  { *((size_t *) data) += 1; }