#include <frags/frag_nsm.h>
#include <operators/scan.h>
#include <field.h>
#include <schema.h>
#include <containers/vector.h>
#include <attr.h>

static inline void this_fragment_nsm_dipose(frag_t *self);
static inline tuplet_t *this_fragment_nsm_open(frag_t *self);
static inline tuplet_t *this_fragment_insert(struct frag_t *self, size_t ntuplets);

static inline void tuplet_rebase(tuplet_t *tuplet, frag_t *frag, size_t slot_id);
static inline tuplet_t *fragment_nsm_open_internal(frag_t *self, size_t pos);

static inline bool this_tuplet_next(tuplet_t *self);
static inline field_t *this_tuplet_open(tuplet_t *self);
static inline void this_tuplet_update(tuplet_t *self, const void *data);
static inline void this_tuplet_set_null(tuplet_t *self);
static inline void this_tuplet_delete(tuplet_t *self);
static inline void this_tuplet_close(tuplet_t *self);
static inline bool this_tuplet_is_null(tuplet_t *self);

static inline bool this_field_next(field_t *self);
static inline const void *this_field_read(field_t *self);
static inline void this_field_update(field_t *self, const void *data);
static inline void this_field_set_null(field_t *self);
static inline bool this_field_is_null(field_t *self);
static inline void this_field_close(field_t *self);

static inline void field_rebase(field_t *field, tuplet_t *tuplet);

frag_t *gs_fragment_nsm_alloc(schema_t *schema, size_t tuplet_capacity)
{
    frag_t *fragment = malloc(sizeof(frag_t));
    size_t tuplet_size   = gs_tuplet_size_by_schema(schema);
    size_t required_size = tuplet_size * tuplet_capacity;
    *fragment = (frag_t) {
            .schema = schema,
            .format = TF_NSM,
            .ntuplets = 0,
            .ncapacity = tuplet_capacity,
            .tuplet_data = require_good_malloc (required_size),
            .tuplet_size = tuplet_size,
            ._scan = scan_mediator,
            ._dispose = this_fragment_nsm_dipose,
            ._open = this_fragment_nsm_open,
            ._insert = this_fragment_insert
    };
    return fragment;
}

void this_fragment_nsm_dipose(frag_t *self)
{
    free (self->tuplet_data);
    free (self);
}

static inline void tuplet_rebase(tuplet_t *tuplet, frag_t *frag, size_t slot_id)
{
    assert (tuplet);
    tuplet->slot_id = slot_id;
    tuplet->fragment = frag;
    tuplet->attr_base = frag->tuplet_data + (slot_id * frag->tuplet_size);
}

static inline tuplet_t *fragment_nsm_open_internal(frag_t *self, size_t pos)
{
    tuplet_t *result = NULL;
    if (self->ntuplets > 0) {
        result = require_good_malloc(sizeof(tuplet_t));
        *result = (tuplet_t) {
            ._next = this_tuplet_next,
            ._open = this_tuplet_open,
            ._update = this_tuplet_update,
            ._set_null = this_tuplet_set_null,
            ._delete = this_tuplet_delete,
            ._close = this_tuplet_close,
            ._is_null = this_tuplet_is_null
        };
        tuplet_rebase(result, self, pos);
    }
    return result;
}

tuplet_t *this_fragment_nsm_open(frag_t *self)
{
    return fragment_nsm_open_internal(self, 0);
}

static inline tuplet_t *this_fragment_insert(struct frag_t *self, size_t ntuplets)
{
    assert (self);
    assert (ntuplets > 0);
    size_t new_size = (self->ntuplets + ntuplets);
    size_t return_tuplet_id = self->ntuplets;
    if (new_size > self->ncapacity) {
        size_t new_capacity = self->ncapacity;
        while (new_capacity < new_size) {
            new_capacity = max(1, ceil(new_capacity * 1.7f));
        }
        self->tuplet_data = realloc(self->tuplet_data, new_capacity * self->tuplet_size);
        self->ncapacity = new_capacity;
    }
    self->ntuplets += ntuplets;
    return fragment_nsm_open_internal(self, return_tuplet_id);
}

// - T U P L E T   I M P L E M E N T A T I O N -------------------------------------------------------------------------

static inline bool this_tuplet_next(tuplet_t *self)
{
    assert (self);
    size_t next_pos = self->slot_id + 1;
    if (next_pos < self->fragment->ntuplets) {
        tuplet_rebase(self, self->fragment, next_pos);
        return true;
    } else {
        gs_tuplet_close(self);
        return false;
    }
}

static inline void field_rebase(field_t *field, tuplet_t *tuplet) {
    field->attr_id = 0;
    field->tuplet = tuplet;
    field->attr_value_ptr = tuplet->attr_base;
}

static inline field_t *this_tuplet_open(tuplet_t *self)
{
    assert (self);
    assert (self->fragment);
    assert (self->fragment->ntuplets);

    field_t *result = require_good_malloc(sizeof(field_t));
    *result = (field_t) {
        ._next = this_field_next,
        ._read = this_field_read,
        ._update = this_field_update,
        ._set_null = this_field_set_null,
        ._is_null = this_field_is_null,
        ._close = this_field_close
    };
    field_rebase(result, self);
    return result;
}

static inline void this_tuplet_update(tuplet_t *self, const void *data)
{
    assert (self);
    assert (data);
    memcpy(self->attr_base, data, self->fragment->tuplet_size);
}

static inline void this_tuplet_set_null(tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, "null values are currently not supported");
}

static inline void this_tuplet_delete(tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, "tuplet delete requests are currently not supported");
}

static inline void this_tuplet_close(tuplet_t *self)
{
    assert (self != NULL);
    free (self);
}

static inline bool this_tuplet_is_null(tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(this_tuplet_is_null));
    return false;
}

// - F I E L D   I M P L E M E N T A T I O N ---------------------------------------------------------------------------

static inline bool this_field_next(field_t *self)
{
    assert (self);
    const attr_id_t next_attr_id = self->attr_id + 1;
    if (next_attr_id < self->tuplet->fragment->schema->attr->num_elements) {
        size_t skip_size = gs_field_size(self);
        self->attr_value_ptr += skip_size;
        self->attr_id = next_attr_id;
        return true;
    } else {
        bool valid_tuplet = gs_tuplet_next(self->tuplet);
        if (valid_tuplet) {
            field_rebase(self, self->tuplet);
            return true;
        } else {
            gs_field_close(self);
            return false;
        }
    }
}

static inline const void *this_field_read(field_t *self)
{
    assert (self);
    return self->attr_value_ptr;
}

static inline void this_field_update(field_t *self, const void *data)
{
    assert (self && data);
    memcpy(self->attr_value_ptr, data, gs_field_size(self));
}

static inline void this_field_set_null(field_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(this_field_set_null));
}

static inline bool this_field_is_null(field_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(this_field_set_null));
    return false;
}

static inline void this_field_close(field_t *self)
{
    assert (self);
    free (self);
}