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

#include <frags/gs_frag_host_vm.h>
#include <operators/gs_scan.h>
#include <gs_tuplet_field.h>
#include <gs_schema.h>
#include <containers/gs_vec.h>
#include <gs_attr.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define REQUIRE_VALID_TUPLET_FORMAT(format)                                                                            \
    REQUIRE((format == TF_NSM || format == TF_DSM), "unknown tuplet serialization format")

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

 gs_frag_t *frag_create(gs_schema_t *schema, size_t tuplet_capacity, enum gs_tuplet_format_e format);

 void frag_open(gs_tuplet_t *dst, gs_frag_t *self, gs_tuplet_id_t tuplet_id);
 void frag_add(gs_tuplet_t *dst, struct gs_frag_t *self, size_t ntuplets);
 void frag_dipose(gs_frag_t *self);

 void tuplet_rebase(gs_tuplet_t *tuplet, gs_frag_t *frag, gs_tuplet_id_t tuplet_id);
 bool tuplet_step(gs_tuplet_t *self);
 void frag_open_internal(gs_tuplet_t *out, gs_frag_t *self, size_t pos);
 void tuplet_bind(gs_tuplet_field_t *dst, gs_tuplet_t *self);
 void tuplet_set_value(gs_tuplet_t *self, const void *data);
 void tuplet_set_null2(gs_tuplet_t *self);
 void tuplet_delete(gs_tuplet_t *self);
 bool tuplet_is_null2(gs_tuplet_t *self);

 void field_rebase(gs_tuplet_field_t *field);
 void field_movebase(gs_tuplet_field_t *field);
 size_t field_nsm_jmp_size(gs_tuplet_field_t *field);
 size_t field_dsm_jmp_size(gs_tuplet_field_t *field, size_t dst_tuplet_slot_id, size_t dst_attr_id);
 bool field_next(gs_tuplet_field_t *field, bool auto_next);
 bool field_seek(gs_tuplet_field_t *field, gs_attr_id_t attr_id);
 const void *field_read(gs_tuplet_field_t *field);
 void field_update(gs_tuplet_field_t *field, const void *data);
 void field_set_null(gs_tuplet_field_t *field);
 bool field_is_null(gs_tuplet_field_t *field);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

struct gs_frag_t *gs_frag_host_vm_nsm_new(gs_schema_t *schema, size_t tuplet_capacity)
{
    return frag_create(schema, tuplet_capacity, TF_NSM);
}

struct gs_frag_t *gs_frag_host_vm_dsm_new(gs_schema_t *schema, size_t tuplet_capacity)
{
    return frag_create(schema, tuplet_capacity, TF_DSM);
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

 gs_frag_t *frag_create(gs_schema_t *schema, size_t tuplet_capacity, enum gs_tuplet_format_e format)
{
    gs_frag_t *fragment = GS_REQUIRE_MALLOC(sizeof(gs_frag_t));
    size_t tuplet_size   = gs_tuplet_size_by_schema(schema);
    size_t required_size = tuplet_size * tuplet_capacity;
    *fragment = (gs_frag_t) {
            .schema = gs_schema_cpy(schema),
            .format = format,
            .ntuplets = 0,
            .ncapacity = tuplet_capacity,
            .tuplet_data = GS_REQUIRE_MALLOC (required_size),
            .tuplet_size = tuplet_size,
            ._scan = gs_scan_mediator,
            ._dispose = frag_dipose,
            ._open = frag_open,
            ._insert = frag_add
    };
    return fragment;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

// - F R A G M E N T   I M P L E M E N T A T I O N ---------------------------------------------------------------------

void frag_dipose(gs_frag_t *self)
{
    free (self->tuplet_data);
    gs_schema_delete(self->schema);
    free (self);
}

 void tuplet_rebase(gs_tuplet_t *tuplet, gs_frag_t *frag, gs_tuplet_id_t tuplet_id)
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

 void frag_open_internal(gs_tuplet_t *out, gs_frag_t *self, size_t pos)
{
    GS_REQUIRE_NONNULL(out)
    if (self->ntuplets > 0) {
        *out = (gs_tuplet_t) {
            ._next = tuplet_step,
            ._open = tuplet_bind,
            ._update = tuplet_set_value,
            ._set_null = tuplet_set_null2,
            ._delete = tuplet_delete,
            ._is_null = tuplet_is_null2
        };
        tuplet_rebase(out, self, pos);
    }
}

void frag_open(gs_tuplet_t *dst, gs_frag_t *self, gs_tuplet_id_t tuplet_id)
{
    frag_open_internal(dst, self, tuplet_id);
}

 void frag_add(gs_tuplet_t *dst, struct gs_frag_t *self, size_t ntuplets)
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

 bool tuplet_step(gs_tuplet_t *self)
{
    assert (self);
    gs_tuplet_id_t next_tuplet_id = self->tuplet_id + 1;
    if (next_tuplet_id < self->fragment->ntuplets) {
        tuplet_rebase(self, self->fragment, next_tuplet_id);
        return true;
    } else {
        return false;
    }
}

 void field_rebase(gs_tuplet_field_t *field)
{
    field->attr_id = 0;
    field->attr_value_ptr = field->tuplet->attr_base;
}

 void field_movebase(gs_tuplet_field_t *field)
{
    enum gs_tuplet_format_e format = field->tuplet->fragment->format;


    size_t skip_size = (format == TF_NSM ?
                        field_nsm_jmp_size(field) :
                        field_dsm_jmp_size(field, field->tuplet->tuplet_id, (field->attr_id + 1)));

    field->attr_value_ptr = (format == TF_NSM ? field->attr_value_ptr : field->tuplet->fragment->tuplet_data) +
                           skip_size;

    field->attr_id++;
}

 void tuplet_bind(gs_tuplet_field_t *dst, gs_tuplet_t *self)
{
    GS_REQUIRE_NONNULL(dst)
    assert (self);
    assert (self->fragment);
    assert (self->fragment->ntuplets);

    *dst = (gs_tuplet_field_t) {
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

 void tuplet_set_value(gs_tuplet_t *self, const void *data)
{
    assert (self);
    assert (data);
    memcpy(self->attr_base, data, self->fragment->tuplet_size);
}

 void tuplet_set_null2(gs_tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, "null values are currently not supported");
}

 void tuplet_delete(gs_tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, "tuplet delete requests are currently not supported");
}

 bool tuplet_is_null2(gs_tuplet_t *self)
{
    assert (self);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(tuplet_is_null2));
    return false;
}

// - F I E L D   I M P L E M E N T A T I O N ---------------------------------------------------------------------------

 size_t field_nsm_jmp_size(gs_tuplet_field_t *field)
{
    return gs_tuplet_field_size(field);
}

 size_t field_dsm_jmp_size(gs_tuplet_field_t *field, size_t dst_tuplet_slot_id, size_t dst_attr_id)
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

 bool field_next(gs_tuplet_field_t *field, bool auto_next)
{
    assert (field);
    assert (field->tuplet->fragment);

    enum gs_tuplet_format_e format = field->tuplet->fragment->format;
    REQUIRE_VALID_TUPLET_FORMAT(format);

    const gs_attr_id_t next_attr_id = field->attr_id + 1;
    if (next_attr_id < field->tuplet->fragment->schema->attr->num_elements) {
        field_movebase(field);
        return true;
    } else {
        if (auto_next && tuplet_step(field->tuplet)) {
              field_rebase(field);
              return true;
        }
        return false;
    }
}

 bool field_seek(gs_tuplet_field_t *field, gs_attr_id_t attr_id)
{
    bool result = true;
    while(attr_id-- && result)
        result &= field_next(field, false);
    return result;
}

 const void *field_read(gs_tuplet_field_t *field)
{
    assert (field);
    return field->attr_value_ptr;
}

 void field_update(gs_tuplet_field_t *field, const void *data)
{
    assert (field && data);
    const gs_attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    if (gs_attr_isstring(attr)) {
        const char *str = *(const char **) data;
        strcpy(field->attr_value_ptr, str);
    } else {
        memcpy(field->attr_value_ptr, data, gs_tuplet_field_size(field));
    }
}

 void field_set_null(gs_tuplet_field_t *field)
{
    assert (field);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(field_set_null));
}

 bool field_is_null(gs_tuplet_field_t *field)
{
    assert (field);
    // TODO: Implement
    panic(NOTIMPLEMENTED, to_string(field_set_null));
    return false;
}