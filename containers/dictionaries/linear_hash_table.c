// An implementation of a open addressing hash table data structure with linear probing
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

#include <containers/dictionaries/linear_hash_table.h>
#include <require.h>
#include <msg.h>
#include <limits.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define BADTAG "dictionary is not tagged as linear hash table instance."

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define seek_to_slot(base, id, key_size, elem_size)                                                                    \
    (base + id * (key_size + elem_size))

#define is_slot_empty(slot_data, key_size, empty_indicator)                                                            \
    ({                                                                                                                 \
        bool empty = true;                                                                                             \
        for (size_t byte_idx = 0; byte_idx < key_size; byte_idx++) {                                                   \
            empty &= (*(char *)(slot_data + byte_idx) == *(char *)(empty_indicator + byte_idx));                       \
        }                                                                                                              \
        empty;                                                                                                         \
    })

#define require_linear_hash_table_tag(dict)                                                                            \
    require((dict->tag == dict_type_linear_hash_table), BADTAG);

#define require_instanceof_this(dict)                                                                                  \
    { require_nonnull(dict); require_nonnull(dict->extra); require_linear_hash_table_tag(dict); }

#define shallow_cpy(other)                                                                                             \
    ({(hash_table_extra_t) {                                                                                           \
        .mutex = (other)->mutex,                                                                                       \
        .num_inuse = (other)->num_inuse,                                                                               \
        .empty_indicator = (other)->empty_indicator,                                                                   \
        .grow_factor = (other)->grow_factor,                                                                           \
        .hash = (other)->hash,                                                                                         \
        .hash_code = (other)->hash_code,                                                                               \
        .num_slots = (other)->num_slots,                                                                               \
        .slots = (other)->slots                                                                                        \
    };})

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    hash_code_fn_t hash_code;
    hash_fn_t hash;
    void *slots;
    size_t num_slots, num_inuse;
    float grow_factor;
    void *empty_indicator;
    pthread_mutex_t mutex;
    counters_t counters;
} hash_table_extra_t;

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void this_clear(struct dictionary_t *self);
bool this_empty(const struct dictionary_t *self);
bool this_contains_values(const struct dictionary_t *self, size_t num_values, const void *values);
bool this_contains_keys(const struct dictionary_t *self, size_t num_keys, const void *keys);
const void *this_get(const struct dictionary_t *self, const void *key);
vector_t *this_gets(const struct dictionary_t *self, size_t num_keys, const void *keys);
bool this_remove(struct dictionary_t *self, size_t num_keys, const void *keys);
void this_put(struct dictionary_t *self, const void *key, const void *value);
void this_puts(struct dictionary_t *self, size_t num_elements, const void *keys, const void *values);
size_t this_num_elements(struct dictionary_t *self);
void this_for_each(struct dictionary_t *self, void *capture, void (*consumer)(void *capture, const void *key, const void *value));

static inline void init_slots(void *slots, size_t num_slots, size_t key_size, size_t elem_size,
                              void *empty_indicator);
static inline bool slot_is_empty(void *slots, size_t slot_id, size_t key_size, size_t elem_size,
                                 void *empty_indicator);
static inline void slot_put(void *slots, size_t slot_id, size_t key_size, size_t elem_size, const void *key, const void *data);
//static inline void *slot_get(void *slots, size_t slot_id, size_t key_size, size_t elem_size);

static inline void rebuild(dictionary_t *self, hash_table_extra_t *extra);
static inline void swap(hash_table_extra_t *a, hash_table_extra_t *b);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

dictionary_t *linear_hash_table_create(hash_code_fn_t hash_code, hash_fn_t hash, size_t key_size, size_t elem_size,
                                       size_t num_slots, float grow_factor)
{
    require_nonnull(hash_code);
    require_nonnull(hash);
    require_not_zero(num_slots);

    dictionary_t *result = require_good_malloc(sizeof(dictionary_t));
    *result = (dictionary_t) {
        .tag = dict_type_linear_hash_table,
        .key_size = key_size,
        .elem_size = elem_size,

        .clear = this_clear,
        .empty = this_empty,
        .contains_values = this_contains_values,
        .contains_keys = this_contains_keys,
        .get = this_get,
        .gets = this_gets,
        .remove = this_remove,
        .put = this_put,
        .puts = this_puts,
        .num_elements = this_num_elements,
        .for_each = this_for_each,

        .extra = require_good_malloc(sizeof(hash_table_extra_t))
    };

    hash_table_extra_t *extra = (hash_table_extra_t*) result->extra;
    *extra = (hash_table_extra_t) {
        .hash = hash,
        .hash_code = hash_code,
        .num_slots = num_slots,
        .num_inuse = 0,
        .grow_factor = grow_factor,
        .slots = require_good_malloc(num_slots * (key_size + elem_size)),
        .empty_indicator = require_good_malloc(key_size),
        .mutex = PTHREAD_MUTEX_INITIALIZER
    };
    memset(extra->empty_indicator, UCHAR_MAX, key_size);

    init_slots(extra->slots, num_slots, key_size, elem_size, extra->empty_indicator);
    linear_hash_reset_counters(result);

    return result;
}

void linear_hash_table_lock(dictionary_t *dict)
{
    require_instanceof_this(dict);
    hash_table_extra_t* extra = (hash_table_extra_t*) dict->extra;
    pthread_mutex_lock(&(extra->mutex));
    extra->counters.num_locks++;
}

void linear_hash_table_unlock(dictionary_t *dict)
{
    require_instanceof_this(dict);
    pthread_mutex_unlock(&(((hash_table_extra_t*) dict->extra)->mutex));
}

void linear_hash_table_free(dictionary_t *dict)
{
    require_instanceof_this(dict);
    hash_table_extra_t *extra = (hash_table_extra_t *) dict->extra;
    require_nonnull(extra->slots);
    require_nonnull(extra->empty_indicator);
    free (extra->slots);
    free (extra->empty_indicator);
    free (dict);
}

void linear_hash_reset_counters(dictionary_t *dict)
{
    require_instanceof_this(dict);
    hash_table_extra_t *extra = (hash_table_extra_t *) dict->extra;
    extra->counters = (counters_t) {
        .num_collisions = 0,
        .num_locks      = 0,
        .num_put_calls  = 0,
        .num_rebuilds   = 0
    };
}

void linear_hash_table_info(dictionary_t *dict, linear_hash_table_info_t *info)
{
    require_instanceof_this(dict);
    hash_table_extra_t *extra = (hash_table_extra_t *) dict->extra;

    *info = (linear_hash_table_info_t) {
        .counters        = extra->counters,
        .num_slots_inuse = extra->num_inuse,
        .num_slots_free  = (extra->num_slots - extra->num_inuse),
        .load_factor     = extra->num_inuse /(double) extra->num_slots,
        .user_data_size  = ((dict->key_size + dict->elem_size) * extra->num_inuse),
        .overhead_size   = (sizeof(dict) + sizeof(hash_table_extra_t) +
                            dict->key_size /* empty indicator */ +
                            ((dict->key_size + dict->elem_size) * (extra->num_slots - extra->num_inuse)) /* reserved */)
    };
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void this_clear(struct dictionary_t *self)
{
    require_instanceof_this(self);
    hash_table_extra_t *extra = (hash_table_extra_t *) self->extra;
    extra->num_inuse = 0;
    init_slots(extra->slots, extra->num_slots, self->key_size, self->elem_size, extra->empty_indicator);
}

bool this_empty(const struct dictionary_t *self)
{
    require_instanceof_this(self);
    return (((hash_table_extra_t *) self->extra)->num_inuse == 0);
}

bool this_contains_values(const struct dictionary_t *self, size_t num_values, const void *values)
{
    require_instanceof_this(self);

    return false;
}

bool this_contains_keys(const struct dictionary_t *self, size_t num_keys, const void *keys)
{
    require_instanceof_this(self);

    return false;
}

const void *this_get(const struct dictionary_t *self, const void *key)
{
    require_instanceof_this(self);

    return NULL;
}

vector_t *this_gets(const struct dictionary_t *self, size_t num_keys, const void *keys)
{
    require_instanceof_this(self);

    return NULL;
}

bool this_remove(struct dictionary_t *self, size_t num_keys, const void *keys)
{
    require_instanceof_this(self);

    return false;
}

void this_put(struct dictionary_t *self, const void *key, const void *value)
{
    require_instanceof_this(self);
    require_nonnull(key && value);
    this_puts(self, 1, key, value);
}

void this_puts(struct dictionary_t *self, size_t num_elements, const void *keys, const void *values)
{
    require_instanceof_this(self);
    require_nonnull(keys && values);
    require_not_zero(num_elements);

    hash_table_extra_t *extra = (hash_table_extra_t *) self->extra;

    while (num_elements--) {
        const void *key   = keys + (num_elements * self->key_size);
        const void *value = values + (num_elements * self->elem_size);

        size_t hash_code = extra->hash_code(self->key_size, key);
        size_t slot_id = extra->hash(hash_code, extra->num_slots);
        size_t round_trip_slot_id = (slot_id == 0 ? (extra->num_slots - 1) : ((slot_id - 1) % extra->num_slots));

        while ((round_trip_slot_id != slot_id) && !slot_is_empty(extra->slots, slot_id, self->key_size,
                                                                 self->elem_size, extra->empty_indicator)) {
            slot_id = ((slot_id + 1) % extra->num_slots);
            extra->counters.num_collisions++;
        }

        if (round_trip_slot_id == slot_id) {
            rebuild(self, extra);
            extra->counters.num_rebuilds++;
            this_put(self, key, value);
        } else {
            slot_put(extra->slots, slot_id, self->key_size, self->elem_size, key, value);
            assert (extra->num_inuse < extra->num_slots);
            extra->num_inuse++;
            extra->counters.num_put_calls++;
        }
    }
}

size_t this_num_elements(struct dictionary_t *self)
{
    require_instanceof_this(self);
    return ((hash_table_extra_t *) self->extra)->num_inuse;
}

void this_for_each(struct dictionary_t *self, void *capture, void (*consumer)(void *capture, const void *key, const void *value))
{
    require_instanceof_this(self);
    hash_table_extra_t * extra = (hash_table_extra_t *) self->extra;
    size_t slot_idx = extra->num_slots;
    size_t slot_size = (self->key_size + self->elem_size);

    while (slot_idx--) {
        void *slot_data = (extra->slots + (slot_idx * slot_size));
        if (!is_slot_empty(slot_data, self->key_size, extra->empty_indicator))
            consumer (capture, slot_data, (slot_data + self->key_size));
    }
}

static inline void init_slots(void *slots, size_t num_slots, size_t key_size, size_t elem_size,
                              void *empty_indicator)
{
    assert (slots && num_slots > 0 && key_size > 0);

    size_t slot_len = (key_size + elem_size);
    while (num_slots--) {
        memcpy(slots + (num_slots * slot_len), empty_indicator, key_size);
    }
}

static inline bool slot_is_empty(void *slots, size_t slot_id, size_t key_size, size_t elem_size,
                                 void *empty_indicator)
{
    assert (slots && empty_indicator && key_size > 0 && elem_size > 0);
    void *pos = seek_to_slot(slots, slot_id, key_size, elem_size);
    bool empty = is_slot_empty(pos, key_size, empty_indicator);

    return empty;
}

static inline void slot_put(void *slots, size_t slot_id, size_t key_size, size_t elem_size, const void *key, const void *data)
{
    assert (slots && key && data && key_size > 0 && elem_size > 0);
    void *slot_data = seek_to_slot(slots, slot_id, key_size, elem_size);
    memcpy(slot_data, key, key_size);
    memcpy(slot_data + key_size, data, elem_size);
}

//static inline void *slot_get(void *slots, size_t slot_id, size_t key_size, size_t elem_size)
//{
//    assert (slots && key_size > 0 && elem_size > 0);
//    return seek_to_slot(slots, slot_id, key_size, elem_size) + key_size;
//}

static inline void rebuild(dictionary_t *self, hash_table_extra_t *extra)
{
    assert (self && extra && extra->grow_factor > 1);

    size_t new_num_slots = ceil(extra->num_slots * extra->grow_factor);
    dictionary_t *tmp = linear_hash_table_create(extra->hash_code, extra->hash, self->key_size, self->elem_size,
                                                 new_num_slots, extra->grow_factor);

    size_t slot_idx = extra->num_slots;
    size_t slot_size = (self->key_size + self->elem_size);

    while (slot_idx--) {
        void *old_slot_data = (extra->slots + (slot_idx * slot_size));
        if (!is_slot_empty(old_slot_data, self->key_size, extra->empty_indicator)) {
            tmp->put(tmp, old_slot_data, old_slot_data + self->key_size);
        }
    }

    swap((hash_table_extra_t *) tmp->extra, extra);
    linear_hash_table_free(tmp);
}

static inline void swap(hash_table_extra_t *a, hash_table_extra_t *b)
{
    hash_table_extra_t tmp = shallow_cpy(a);
    *a = shallow_cpy(b);
    *b = shallow_cpy(&tmp);
}