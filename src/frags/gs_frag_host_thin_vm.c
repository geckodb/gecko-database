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

#include <gecko-commons/containers/gs_vec.h>

#include <frags/gs_frag_host_vm.h>
#include <operators/gs_scan.h>
#include <gs_tuplet_field.h>
#include <gs_schema.h>
#include <gs_attr.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

//typedef struct gs_frag_thin_extras {
//    gs_hash_t *gs_hash_t;
//} gs_frag_thin_extras;


// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define REQUIRE_VALID_TUPLET_FORMAT(format)                                                                            \
    REQUIRE((format == TF_NSM || format == TF_DSM), "unknown tuplet serialization format")

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

 static gs_frag_t *frag_create(gs_schema_t *schema, size_t tuplet_capacity, enum gs_tuplet_format_e format);

 static void frag_open(gs_tuplet_t *dst, gs_frag_t *self, gs_tuplet_id_t tuplet_id);
 static void frag_add(gs_tuplet_t *dst, struct gs_frag_t *self, size_t ntuplets);
 static void frag_dipose(gs_frag_t *self);

 static void tuplet_rebase(gs_tuplet_t *tuplet, gs_frag_t *frag, gs_tuplet_id_t tuplet_id);
 static bool tuplet_step(gs_tuplet_t *self);
 static void frag_open_internal(gs_tuplet_t *out, gs_frag_t *self, size_t pos);
 static void tuplet_bind(gs_tuplet_field_t *dst, gs_tuplet_t *self);
 static void tuplet_set_null2(gs_tuplet_t *self);
 static void tuplet_delete(gs_tuplet_t *self);
 static bool tuplet_is_null2(gs_tuplet_t *self);

 static void field_rebase(gs_tuplet_field_t *field);
 static void field_movebase(gs_tuplet_field_t *field);
 static bool field_next(gs_tuplet_field_t *field, bool auto_next);
 static bool field_seek(gs_tuplet_field_t *field, gs_attr_id_t attr_id);
 static const void *field_read(gs_tuplet_field_t *field);
 static void field_update(gs_tuplet_field_t *field, const void *data);
 static void field_set_null(gs_tuplet_field_t *field);
 static bool field_is_null(gs_tuplet_field_t *field);
 static inline int gs_comp_func_attr(const void *lhs, const void *rhs);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

struct gs_frag_t *gs_frag_host_vm_thin_dsm_new(gs_schema_t *schema, size_t tuplet_capacity)
{
    return frag_create(schema, tuplet_capacity, TF_DSM);
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

 gs_frag_t *frag_create(gs_schema_t *schema, size_t tuplet_capacity, enum gs_tuplet_format_e format)
{
    gs_frag_t *fragment = GS_REQUIRE_MALLOC(sizeof(gs_frag_t));
    gs_frag_thin_extras thin_extras;
    *fragment = (gs_frag_t) {
            .schema = gs_schema_cpy(schema),
            .format = format,
            .ntuplets = 0,
            .ncapacity = tuplet_capacity,
            .extras = &thin_extras,
            ._scan = gs_scan_mediator,
            ._dispose = frag_dipose,
            ._open = frag_open,
            ._insert = frag_add
    };

    // create the dictionary to match attribute ids to vectors
    // loop to the number of attributes initiate the attributes once.
    gs_hash_create(&(((gs_frag_thin_extras *) fragment->extras)->gs_hash_t)
            , 10, gs_comp_func_attr);
    for (size_t attr_id = 0; attr_id < fragment->schema->attr->num_elements; ++attr_id) {
        const gs_attr_t *attr = gs_schema_attr_by_id(fragment->schema, attr_id);
        size_t attr_total_size = gs_attr_total_size(attr);
        gs_hash_set((((gs_frag_thin_extras *) fragment->extras))->gs_hash_t,
                    attr, attr_total_size,
                    gs_vec_new(attr_total_size, tuplet_capacity));
    }

    return fragment;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

// - F R A G M E N T   I M P L E M E N T A T I O N ---------------------------------------------------------------------

void frag_dipose(gs_frag_t *self)
{

    for (size_t attr_id = 0; attr_id < self->schema->attr->num_elements; ++attr_id) {
        const gs_attr_t *attr = gs_schema_attr_by_id(self->schema, attr_id);
        size_t attr_total_size = gs_attr_total_size(attr);
        gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) self->extras))->gs_hash_t,
                                          attr, attr_total_size);
        gs_vec_dispose(attr_vals);
    }
    gs_hash_dispose((((gs_frag_thin_extras *) self->extras))->gs_hash_t);
    gs_schema_delete(self->schema);
    free (self);
}

 void tuplet_rebase(gs_tuplet_t *tuplet, gs_frag_t *frag, gs_tuplet_id_t tuplet_id)
{
    assert (tuplet);
    tuplet->tuplet_id = tuplet_id;
    tuplet->fragment = frag;
}

 void frag_open_internal(gs_tuplet_t *out, gs_frag_t *self, size_t pos)
{
    GS_REQUIRE_NONNULL(out)
    if (self->ntuplets > 0) {
        *out = (gs_tuplet_t) {
            ._next = tuplet_step,
            ._open = tuplet_bind,
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
    assert (self);
    assert (ntuplets > 0);
    size_t return_tuplet_id = self->ntuplets;
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
}

 void field_movebase(gs_tuplet_field_t *field)
{
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
    const gs_attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);

    size_t attr_total_size = gs_attr_total_size(attr);

    gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) field->tuplet->fragment->extras))->gs_hash_t,
                                      attr, attr_total_size);
    return gs_vec_at(attr_vals, field->tuplet->tuplet_id);
}

 void field_update(gs_tuplet_field_t *field, const void *data)
{
    assert (field && data);
    const gs_attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    size_t attr_total_size = gs_attr_total_size(attr);
    gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) field->tuplet->fragment->extras))->gs_hash_t,
                                      attr, attr_total_size);
    if (gs_attr_isstring(attr)) {
        const char *str = *(const char **) data;
        gs_vec_pushback(attr_vals, 1, str);

    } else {
        gs_vec_pushback(attr_vals, 1, data);
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

static inline int gs_comp_func_attr(const void *lhs, const void *rhs)
{
    gs_attr_id_t a = ((gs_attr_t *) lhs)->id;
    gs_attr_id_t b = ((gs_attr_t *) rhs)->id;
    if (a < b) {
        return -1;
    } else if (a > b) {
        return +1;
    } else return 0;
}