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

#include <frags/frag_host_vm.h>
#include <operators/scan.h>
#include <tuplet_field.h>
#include <schema.h>
#include <containers/vector.h>
#include <attr.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define REQUIRE_VALID_TUPLET_FORMAT(format)                                                                            \
    REQUIRE((format == TF_NSM || format == TF_DSM), "unknown tuplet serialization format")

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline frag_t *frag_create(schema_t *schema, size_t tuplet_capacity, enum tuplet_format format);

static inline void frag_open(tuplet_t *dst, frag_t *self, tuplet_id_t tuplet_id);
static inline void frag_insert(tuplet_t *dst, struct frag_t *self, size_t ntuplets);
static inline void frag_dipose(frag_t *self);

static inline void tuplet_rebase(tuplet_t *tuplet, frag_t *frag, tuplet_id_t tuplet_id);
static inline bool tuplet_next(tuplet_t *self);
static inline void frag_open_internal(tuplet_t *out, frag_t *self, size_t pos);
static inline void tuplet_open(tuplet_field_t *dst, tuplet_t *self);
static inline void tuplet_update(tuplet_t *self, const void *data);
static inline void tuplet_set_null(tuplet_t *self);
static inline void tuplet_delete(tuplet_t *self);
static inline bool tuplet_is_null(tuplet_t *self);

static inline void field_rebase(tuplet_field_t *field);
static inline void field_movebase(tuplet_field_t *field);
static inline size_t field_nsm_jmp_size(tuplet_field_t *field);
static inline size_t field_dsm_jmp_size(tuplet_field_t *field, size_t dst_tuplet_slot_id, size_t dst_attr_id);
static inline bool field_next(tuplet_field_t *field, bool auto_next);
static inline bool field_seek(tuplet_field_t *field, attr_id_t attr_id);
static inline const void *field_read(tuplet_field_t *field);
static inline void field_update(tuplet_field_t *field, const void *data);
static inline void field_set_null(tuplet_field_t *field);
static inline bool field_is_null(tuplet_field_t *field);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

struct frag_t *gs_frag_host_vm_nsm_create(schema_t *schema, size_t tuplet_capacity)
{
    return frag_create(schema, tuplet_capacity, TF_NSM);
}

struct frag_t *gs_frag_host_vm_dsm_create(schema_t *schema, size_t tuplet_capacity)
{
    return frag_create(schema, tuplet_capacity, TF_DSM);
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline frag_t *frag_create(schema_t *schema, size_t tuplet_capacity, enum tuplet_format format)
{
    frag_t *fragment = REQUIRE_MALLOC(sizeof(frag_t));
    size_t tuplet_size   = gs_tuplet_size_by_schema(schema);
    size_t required_size = tuplet_size * tuplet_capacity;
    *fragment = (frag_t) {
            .schema = gs_schema_cpy(schema),
            .format = format,
            .ntuplets = 0,
            .ncapacity = tuplet_capacity,
            .tuplet_data = REQUIRE_MALLOC (required_size),
            .tuplet_size = tuplet_size,
            ._scan = scan_mediator,
            ._dispose = frag_dipose,
            ._open = frag_open,
            ._insert = frag_insert
    };
    return fragment;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

// - F R A G M E N T   I M P L E M E N T A T I O N ---------------------------------------------------------------------

void frag_dipose(frag_t *self)
{
    free (self->tuplet_data);
    gs_schema_free(self->schema);
    free (self);
}

static inline void tuplet_rebase(tuplet_t *tuplet, frag_t *frag, tuplet_id_t tuplet_id)
{
    assert (tuplet);
    REQUIRE_VALID_TUPLET_FORMAT(frag->format);
    REQUIRE((tuplet_id < frag->ntuplets), "Tuplet id out of bounds");

    tuplet->tuplet_id = tuplet_id;
    tuplet->fragment = frag;

    size_t offset = tuplet_id * (frag->format == TF_NSM ?
                               (frag->tuplet_size) :
                               gs_attr_total_size(gs_schema_attr_by_id(frag->schema, 0)));

    tuplet->attr_base = frag->tuplet_data + offset;
}

static inline void frag_open_internal(tuplet_t *out, frag_t *self, size_t pos)
{
    REQUIRE_NONNULL(out)
    if (self->ntuplets > 0) {
        *out = (tuplet_t) {
            ._next = tuplet_next,
            ._open = tuplet_open,
            ._update = tuplet_update,
            ._set_null = tuplet_set_null,
            ._delete = tuplet_delete,
            ._is_null = tuplet_is_null
        };
        tuplet_rebase(out, self, pos);
    }
}

void frag_open(tuplet_t *dst, frag_t *self, tuplet_id_t tuplet_id)
{
    frag_open_internal(dst, self, tuplet_id);
}

static inline void frag_insert(tuplet_t *dst, struct frag_t *self, size_t ntuplets)
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
    frag_open_internal(dst, self, return_tuplet_id);
}

// - T U P L E T   I M P L E M E N T A T I O N -------------------------------------------------------------------------

static inline bool tuplet_next(tuplet_t *self)
{
    assert (self);
    tuplet_id_t next_tuplet_id = self->tuplet_id + 1;
    if (next_tuplet_id < self->fragment->ntuplets) {
        tuplet_rebase(self, self->fragment, next_tuplet_id);
        return true;
    } else {
        return false;
    }
}

static inline void field_rebase(tuplet_field_t *field)
{
    field->attr_id = 0;
    field->attr_value_ptr = field->tuplet->attr_base;
}

static inline void field_movebase(tuplet_field_t *field)
{
    enum tuplet_format format = field->tuplet->fragment->format;


    size_t skip_size = (format == TF_NSM ?
                        field_nsm_jmp_size(field) :
                        field_dsm_jmp_size(field, field->tuplet->tuplet_id, (field->attr_id + 1)));

    field->attr_value_ptr = (format == TF_NSM ? field->attr_value_ptr : field->tuplet->fragment->tuplet_data) +
                           skip_size;

    field->attr_id++;
}

static inline void tuplet_open(tuplet_field_t *dst, tuplet_t *self)
{
    REQUIRE_NONNULL(dst)
    assert (self);
    assert (self->fragment);
    assert (self->fragment->ntuplets);

    *dst = (tuplet_field_t) {
        ._next = field_next,
        ._seek = field_seek,
        ._read = field_read,
        ._update = field_update,
        ._set_null = field_set_null,
        ._is_null = field_is_null
    };
    dst->tuplet = self;
    field_rebase(dst);
}

static inline void tuplet_update(tuplet_t *self, const void *data)
{
    assert (self);
    assert (data);
    memcpy(self->attr_base, data, self->fragment->tuplet_size);
}

static inline void tuplet_set_null(tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, "null values are currently not supported");
}

static inline void tuplet_delete(tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, "tuplet delete requests are currently not supported");
}

static inline bool tuplet_is_null(tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(tuplet_is_null));
    return false;
}

// - F I E L D   I M P L E M E N T A T I O N ---------------------------------------------------------------------------

static inline size_t field_nsm_jmp_size(tuplet_field_t *field)
{
    return gs_tuplet_field_size(field);
}

static inline size_t field_dsm_jmp_size(tuplet_field_t *field, size_t dst_tuplet_slot_id, size_t dst_attr_id)
{
    size_t skip_size = 0;
    for (size_t attr_id = 0; attr_id < dst_attr_id; attr_id++) {
        skip_size += field->tuplet->fragment->ntuplets *
                     gs_attr_total_size(gs_schema_attr_by_id(field->tuplet->fragment->schema, attr_id));
    }
    skip_size += dst_tuplet_slot_id *
            gs_attr_total_size(gs_schema_attr_by_id(field->tuplet->fragment->schema, dst_attr_id));
    return skip_size;
}

static inline bool field_next(tuplet_field_t *field, bool auto_next)
{
    assert (field);
    assert (field->tuplet->fragment);

    enum tuplet_format format = field->tuplet->fragment->format;
    REQUIRE_VALID_TUPLET_FORMAT(format);

    const attr_id_t next_attr_id = field->attr_id + 1;
    if (next_attr_id < field->tuplet->fragment->schema->attr->num_elements) {
        field_movebase(field);
        return true;
    } else {
        if (auto_next && gs_tuplet_next(field->tuplet)) {
              field_rebase(field);
              return true;
        }
        return false;
    }
}

static inline bool field_seek(tuplet_field_t *field, attr_id_t attr_id)
{
    bool result = true;
    while(attr_id-- && result)
        result &= field_next(field, false);
    return result;
}

static inline const void *field_read(tuplet_field_t *field)
{
    assert (field);
    return field->attr_value_ptr;
}

static inline void field_update(tuplet_field_t *field, const void *data)
{
    assert (field && data);
    const attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    if (gs_attr_isstring(attr)) {
        const char *str = *(const char **) data;
        strcpy(field->attr_value_ptr, str);
    } else {
        memcpy(field->attr_value_ptr, data, gs_tuplet_field_size(field));
    }
}

static inline void field_set_null(tuplet_field_t *field)
{
    assert (field);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(field_set_null));
}

static inline bool field_is_null(tuplet_field_t *field)
{
    assert (field);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(field_set_null));
    return false;
}