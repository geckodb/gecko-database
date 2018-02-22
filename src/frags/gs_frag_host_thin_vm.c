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
#include <gecko-commons/stdinc.h>
#include <frags/gs_frag_host_vm.h>
#include <operators/gs_scan.h>
#include <gs_tuplet_field.h>
#include <gs_schema.h>
#include <gs_attr.h>


// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define REQUIRE_VALID_TUPLET_FORMAT(format)                                                                            \
    REQUIRE(format == TF_DSM, "unknown tuplet serialization format")

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

 static gs_frag_t *frag_create(gs_schema_t *schema, size_t tuplet_capacity, enum gs_tuplet_format_e format);

 static void frag_open(gs_tuplet_t *dst, gs_frag_t *self, gs_tuplet_id_t tuplet_id);
 static void frag_add(gs_tuplet_t *dst, struct gs_frag_t *self, size_t ntuplets);
 static void frag_dipose(gs_frag_t *self);
 static void frag_raw_scan(struct gs_frag_t *self, gs_vec_t *match_ids, enum gs_comp_type_e comp_type, gs_attr_id_t attr_id,
    const void *comp_val);
 static void frag_raw_scan_vectorized(struct gs_frag_t *self, gs_batch_consumer_t *consumer,gs_vec_t *match_ids,
                                     enum gs_comp_type_e comp_type, gs_attr_id_t attr_id, const void *cmp_val,
                                     size_t batch_size, bool enable_multithreading);
 static void frag_raw_scan_dsm_vectorized(struct gs_frag_t *self, gs_batch_consumer_t *consumer, gs_vec_t *match_ids, enum gs_comp_type_e comp_type, gs_attr_id_t attr_id,
                                         const void *cmp_val, size_t batch_size);
 static void frag_raw_scan_dsm_vectorized_ml(struct gs_frag_t *self, gs_batch_consumer_t *consumer, gs_vec_t *match_ids, enum gs_comp_type_e comp_type, gs_attr_id_t attr_id,
                                            const void *cmp_val, size_t batch_size);

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
    gs_frag_thin_extras *thin_extras = GS_REQUIRE_MALLOC(sizeof(gs_frag_thin_extras));;
    *fragment = (gs_frag_t) {
            .schema = gs_schema_cpy(schema),
            .format = format,
            .ntuplets = 0,
            .ncapacity = tuplet_capacity,
            .extras = thin_extras,
            ._scan = gs_scan_mediator,
            ._raw_scan = frag_raw_scan,
            ._raw_vectorized_scan = frag_raw_scan_vectorized,
            ._dispose = frag_dipose,
            ._open = frag_open,
            ._insert = frag_add
    };

    // create the dictionary to match attribute ids to vectors
    // loop to the number of attributes initiate the attributes once.
    gs_hash_create(&(((gs_frag_thin_extras *) fragment->extras)->attr_vals_map)
            , 10, gs_comp_func_attr);
    for (size_t attr_id = 0; attr_id < fragment->schema->attr->num_elements; ++attr_id) {
        const gs_attr_t *attr = gs_schema_attr_by_id(fragment->schema, attr_id);
        size_t attr_total_size = gs_attr_total_size(attr);
        gs_hash_set((((gs_frag_thin_extras *) fragment->extras))->attr_vals_map,
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
        gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) self->extras))->attr_vals_map,
                                          attr, attr_total_size);
        gs_vec_dispose(attr_vals);
    }
    gs_hash_dispose((((gs_frag_thin_extras *) self->extras))->attr_vals_map);
    free((((gs_frag_thin_extras *) self->extras)));
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
    assert ((ntuplets > 0));
    assert (self);
    assert ((ntuplets > 0));
    size_t return_tuplet_id = self->ntuplets;
    self->ntuplets += ntuplets;
    frag_open_internal(dst, self, return_tuplet_id);

}
    // operator at a time
 void frag_raw_scan(struct gs_frag_t *self, gs_vec_t *match_ids, enum gs_comp_type_e comp_type, gs_attr_id_t attr_id,
                                     const void *comp_val)
{
    assert(self);
    assert(match_ids);
    assert((attr_id >= 0));
    assert(comp_val);

    const gs_attr_t *attr = gs_schema_attr_by_id(self->schema, attr_id);
    size_t attr_total_size = gs_attr_total_size(attr);
    gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) self->extras))->attr_vals_map,
                                      attr, attr_total_size);
    size_t attr_vals_size = gs_vec_length(attr_vals);

    gs_comp_func_t cmp_func;

    switch (attr->type) {
        case FT_INT32:
            cmp_func = gs_cmp_int32;
            break;
        case FT_UINT32:
            cmp_func = gs_cmp_uint32;
            break;
        case FT_UINT64:
            cmp_func = gs_cmp_uint64;
            break;
        case FT_FLOAT32:
            cmp_func = gs_cmp_float32;
            break;
        case FT_CHAR:
            cmp_func = gs_cmp_char;
            break;
        default: panic("not implemented '%d'", attr->type);
    }

    for (gs_tuplet_id_t tuplet_id = 0; tuplet_id < attr_vals_size; ++tuplet_id) {
            // compare the values and also according to specific function
        const void *field_data = gs_vec_at(attr_vals, tuplet_id);
        int compare = cmp_func(field_data, comp_val);
        bool match = ((comp_type == CT_LESS && compare < 0) ||
                  (comp_type == CT_GREATER && compare > 0) ||
                  (comp_type == CT_EQUALS && compare == 0) ||
                  (comp_type == CT_LESSEQ && compare <= 0) ||
                  (comp_type == CT_GREATEREQ && compare >= 0) ||
                  (comp_type == CT_NOTEQUALS && compare != 0));
        if (match) {
            gs_vec_pushback(match_ids, 1, &tuplet_id);
        }
    }
}

static void frag_raw_scan_vectorized(struct gs_frag_t *self, gs_batch_consumer_t *consumer,gs_vec_t *match_ids,
                                     enum gs_comp_type_e comp_type, gs_attr_id_t attr_id, const void *cmp_val,
                                     size_t batch_size, bool enable_multithreading)
{
    assert(self);
    assert(match_ids);
    assert((attr_id >= 0));
    assert(cmp_val);
    assert((batch_size >= 1));
    if (enable_multithreading) {
        frag_raw_scan_dsm_vectorized_ml(self, consumer, match_ids, comp_type, attr_id, cmp_val, batch_size);
    } else {
        frag_raw_scan_dsm_vectorized(self, consumer, match_ids, comp_type, attr_id, cmp_val, batch_size);
    }

}

static int gs_cmp_uint32_batch(const void *batch, const void *rhs, size_t batch_size, size_t offset_size, size_t attr_skip,
                        short *result)
{
    for (size_t i = 0; i < batch_size; i++) {
        uint32_t* value = (void*)batch + i * offset_size + attr_skip;
        //printf(" %d ", *value);
        if (*value > *(uint32_t *)rhs) result[i] = 1;
        else if (*value < *(uint32_t *)rhs) result[i] = -1;
        else result[i] = 0;

    }
    return 1;
}

static int gs_cmp_int32_batch(const void *batch, const void *rhs, size_t batch_size, size_t offset_size, size_t attr_skip,
                       short *result)
{
    for (size_t i = 0; i < batch_size; i++) {
        int32_t* value = (void*)batch + i * offset_size + attr_skip;
        //printf(" %d ", *value);
        if (*value > *(int32_t *)rhs) result[i] = 1;
        else if (*value < *(int32_t *)rhs) result[i] = -1;
        else result[i] = 0;

    }
    return 1;
}

static int gs_cmp_uint64_batch(const void *batch, const void *rhs, size_t batch_size, size_t offset_size, size_t attr_skip,
                        short *result)
{
    for (size_t i = 0; i < batch_size; i++) {
        uint64_t * value = (void*)batch + i * offset_size + attr_skip;
        //printf(" %d ", *value);
        if (*value > *(uint64_t *)rhs) result[i] = 1;
        else if (*value < *(uint64_t *)rhs) result[i] = -1;
        else result[i] = 0;

    }
    return 1;
}

//TODO: good way to implement comparing two floats in the scenario of batchs
static int gs_cmp_float32_batch(const void *batch, const void *rhs, size_t batch_size, size_t offset_size, size_t attr_skip,
                         short *result)
{
    //   panic(NOTIMPLEMENTED, "need to implement a function to compare floats");
    return 1;
}

//TODO: good way to implement comparing two chars in the scenario of batchs
static int gs_cmp_char_batch(const void *batch, const void *rhs, size_t batch_size, size_t offset_size, size_t attr_skip,
                      short *result)
{
    //   panic(NOTIMPLEMENTED, "need to implement a function to compare two chars");
    return 1;
}

//TODO : Vector Stablization
static void frag_raw_scan_dsm_vectorized(struct gs_frag_t *self, gs_batch_consumer_t *consumer, gs_vec_t *match_ids,
                                         enum gs_comp_type_e comp_type, gs_attr_id_t attr_id, const void *cmp_val,
                                         size_t batch_size)
{
    assert(self);
    assert(match_ids);
    assert((attr_id >= 0));
    assert(cmp_val);
    consumer->new_fragment = true;
    size_t remaining_num_tuplets = self->ntuplets;
    size_t num_batchs = (size_t)ceil(remaining_num_tuplets / (float)(batch_size));

    const gs_attr_t *attr = gs_schema_attr_by_id(self->schema, attr_id);
    size_t attr_total_size = gs_attr_total_size(attr);
    gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) self->extras))->attr_vals_map,
                                      attr, attr_total_size);

    gs_comp_func_batch_t cmp_func;

    switch (attr->type) {
        case FT_INT32:
            cmp_func = gs_cmp_int32_batch;
            break;
        case FT_UINT32:
            cmp_func = gs_cmp_uint32_batch;
            break;
        case FT_UINT64:
            cmp_func = gs_cmp_uint64_batch;
            break;
        case FT_FLOAT32:
            cmp_func = gs_cmp_float32_batch;
            break;
        case FT_CHAR:
            cmp_func = gs_cmp_char_batch;
            break;
        default: panic("not implemented '%d'", attr->type);
    }
    size_t current_offset = 0;
    for (size_t i = 0; i < num_batchs; i++) {
        short *cmp_return = (short*) malloc(sizeof(short) * batch_size);
        size_t proper_batch_size = (remaining_num_tuplets > batch_size)?
                                   batch_size : remaining_num_tuplets;
        remaining_num_tuplets -= proper_batch_size;
        gs_attr_id_t *result = malloc(proper_batch_size * sizeof(gs_attr_id_t));
        cmp_func(gs_vec_at(attr_vals, current_offset), cmp_val, proper_batch_size,
                 attr_vals->sizeof_element, 0, cmp_return);
        size_t match_counter = 0;
        for (gs_tuplet_id_t j = 0; j < proper_batch_size; ++j) {
            bool match = ((comp_type == CT_LESS && cmp_return[j] < 0) ||
                          (comp_type == CT_GREATER && cmp_return[j] > 0) ||
                          (comp_type == CT_EQUALS && cmp_return[j] == 0) ||
                          (comp_type == CT_LESSEQ && cmp_return[j] <= 0) ||
                          (comp_type == CT_GREATEREQ && cmp_return[j] >= 0) ||
                          (comp_type == CT_NOTEQUALS && cmp_return[j] != 0));
            if (match) {
                gs_attr_id_t current_id = current_offset + j;
                gs_vec_pushback(match_ids, 1, &(current_id));
                result[match_counter++] = current_id;
            }
        }
        gs_batch_consumer_consume(consumer, result, self, 0,
                                match_counter);
        current_offset += proper_batch_size;
        free(result);
        free(cmp_return);
    }
}

static void frag_raw_scan_dsm_vectorized_ml(struct gs_frag_t *self, gs_batch_consumer_t *consumer, gs_vec_t *match_ids,
                                            enum gs_comp_type_e comp_type, gs_attr_id_t attr_id, const void *cmp_val,
                                            size_t batch_size)
{
    panic(NOTIMPLEMENTED, "Vectorized multithreaded"
            " scan for Thin DSM isn't yet implemented");
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

    gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) field->tuplet->fragment->extras))->attr_vals_map,
                                      attr, attr_total_size);
    return gs_vec_at(attr_vals, field->tuplet->tuplet_id);
}

 void field_update(gs_tuplet_field_t *field, const void *data)
{
    assert (field && data);
    const gs_attr_t *attr = gs_schema_attr_by_id(field->tuplet->fragment->schema, field->attr_id);
    size_t attr_total_size = gs_attr_total_size(attr);
    gs_vec_t *attr_vals = gs_hash_get((((gs_frag_thin_extras *) field->tuplet->fragment->extras))->attr_vals_map,
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