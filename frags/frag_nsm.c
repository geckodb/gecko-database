#include <frags/frag_nsm.h>
#include <operators/scan.h>
#include <field.h>
#include <schema.h>
#include <containers/vector.h>
#include <attr.h>

static inline void this_fragment_nsm_dipose(fragment_t *self);
static inline tuplet_t *this_fragment_nsm_open(fragment_t *self);
static inline tuplet_t *this_fragment_insert(struct fragment_t *self, size_t ntuplets);

static inline tuplet_t *fragment_nsm_open_internal(fragment_t *self, size_t pos);

static inline tuplet_t *this_tuplet_next(tuplet_t *self);
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

static inline void field_set_values(field_t *field, attr_id_t attr_id, tuplet_t *tuplet, void *attr_base);

fragment_t *gs_fragment_nsm_alloc(schema_t *schema, size_t tuplet_capacity)
{
    fragment_t *fragment = malloc(sizeof(fragment_t));
    *fragment = (fragment_t) {
            .schema = schema,
            .format = TF_NSM,
            .ntuplets = 0,
            .ncapacity = tuplet_capacity,
            .tuplet_data = malloc (gs_tuplet_size_by_schema(schema) * tuplet_capacity),
            .tuplet_size = gs_tuplet_size_by_schema(schema),
            ._scan = scan_mediator,
            ._dispose = this_fragment_nsm_dipose,
            ._open = this_fragment_nsm_open,
            ._insert = this_fragment_insert
    };
    return fragment;
}

void this_fragment_nsm_dipose(fragment_t *self)
{
    free (self->tuplet_data);
    free (self);
}

tuplet_t *fragment_nsm_open_internal(fragment_t *self, size_t pos)
{
    tuplet_t *result = NULL;
    if (self->ntuplets > 0) {
        result = require_good_malloc(sizeof(result));
        *result = (tuplet_t) {
            .id = pos,
            .fragment = self,
            .attr_base = self->tuplet_data,
            ._next = this_tuplet_next,
            ._open = this_tuplet_open,
            ._update = this_tuplet_update,
            ._set_null = this_tuplet_set_null,
            ._delete = this_tuplet_delete,
            ._close = this_tuplet_close,
            ._is_null = this_tuplet_is_null
        };
    }
    return result;
}

tuplet_t *this_fragment_nsm_open(fragment_t *self)
{
    return fragment_nsm_open_internal(self, 0);
}

static inline tuplet_t *this_fragment_insert(struct fragment_t *self, size_t ntuplets)
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

static inline tuplet_t *this_tuplet_next(tuplet_t *self)
{
    assert (self);
    self->attr_base += self->fragment->tuplet_size;
    if (++self->id < self->fragment->ntuplets) {
        self->attr_base = self->fragment->tuplet_data + self->id * self->fragment->tuplet_size;
        return self;
    } else {
        free (self);
        return NULL;
    }
}

static inline void field_set_values(field_t *field, attr_id_t attr_id, tuplet_t *tuplet, void *attr_base) {
    field->attr_id = attr_id;
    field->tuplet = tuplet;
    field->attr_base = attr_base;
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
    field_set_values(result, 0, self, self->attr_base);
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
        self->attr_base += skip_size;
        self->attr_id = next_attr_id;
        return true;
    } else {
        tuplet_t *tuplet = gs_tuplet_next(self->tuplet);
        if (tuplet != NULL) {
            field_set_values(self, 0, tuplet, tuplet->attr_base);
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
    return self->attr_base;
}

static inline void this_field_update(field_t *self, const void *data)
{
    assert (self && data);
    memcpy(self->attr_base, data, gs_field_size(self));
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