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

static inline tuplet_t *frag_nsm_open(frag_t *self, tuplet_id_t tuplet_id);
static inline tuplet_t *frag_insert(struct frag_t *self, size_t ntuplets);
static inline void frag_dipose(frag_t *self);

static inline void tuplet_rebase(tuplet_t *tuplet, frag_t *frag, tuplet_id_t tuplet_id);
static inline bool tuplet_next(tuplet_t *self);
static inline tuplet_t *frag_open_internal(frag_t *self, size_t pos);
static inline tuplet_field_t *tuplet_open(tuplet_t *self);
static inline void tuplet_update(tuplet_t *self, const void *data);
static inline void tuplet_set_null(tuplet_t *self);
static inline void tuplet_delete(tuplet_t *self);
static inline void tuplet_close(tuplet_t *self);
static inline bool tuplet_is_null(tuplet_t *self);

static inline void field_rebase(tuplet_field_t *field, tuplet_t *tuplet);
static inline void field_movebase(tuplet_field_t *field, tuplet_t *tuplet);
static inline size_t field_nsm_jmp_size(tuplet_field_t *field);
static inline size_t field_dsm_jmp_size(tuplet_field_t *field, size_t dst_tuplet_slot_id, size_t dst_attr_id);
static inline bool field_next(tuplet_field_t *field);
static inline bool field_seek(tuplet_field_t *field, attr_id_t attr_id);
static inline const void *field_read(tuplet_field_t *field);
static inline void field_update(tuplet_field_t *field, const void *data);
static inline void field_set_null(tuplet_field_t *field);
static inline bool field_is_null(tuplet_field_t *field);
static inline void field_close(tuplet_field_t *field);

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
            ._open = frag_nsm_open,
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

static inline tuplet_t *frag_open_internal(frag_t *self, size_t pos)
{
    tuplet_t *result = NULL;
    if (self->ntuplets > 0) {
        result = REQUIRE_MALLOC(sizeof(tuplet_t));
        *result = (tuplet_t) {
            ._next = tuplet_next,
            ._open = tuplet_open,
            ._update = tuplet_update,
            ._set_null = tuplet_set_null,
            ._delete = tuplet_delete,
            ._close = tuplet_close,
            ._is_null = tuplet_is_null
        };
        tuplet_rebase(result, self, pos);
    }
    return result;
}

tuplet_t *frag_nsm_open(frag_t *self, tuplet_id_t tuplet_id)
{
    return frag_open_internal(self, tuplet_id);
}

static inline tuplet_t *frag_insert(struct frag_t *self, size_t ntuplets)
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
    return frag_open_internal(self, return_tuplet_id);
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
        gs_tuplet_close(self);
        return false;
    }
}

static inline void field_rebase(tuplet_field_t *field, tuplet_t *tuplet)
{
    field->attr_id = 0;
    field->tuplet = tuplet;
    field->attr_value_ptr = tuplet->attr_base;
}

static inline void field_movebase(tuplet_field_t *field, tuplet_t *tuplet)
{
    enum tuplet_format format = field->tuplet->fragment->format;


    size_t skip_size = (format == TF_NSM ?
                        field_nsm_jmp_size(field) :
                        field_dsm_jmp_size(field, field->tuplet->tuplet_id, (field->attr_id + 1)));

    field->attr_value_ptr = (format == TF_NSM ? field->attr_value_ptr : field->tuplet->fragment->tuplet_data) +
                           skip_size;

    field->attr_id++;
}

static inline tuplet_field_t *tuplet_open(tuplet_t *self)
{
    assert (self);
    assert (self->fragment);
    assert (self->fragment->ntuplets);

    tuplet_field_t *result = REQUIRE_MALLOC(sizeof(tuplet_field_t));
    *result = (tuplet_field_t) {
        ._next = field_next,
        ._seek = field_seek,
        ._read = field_read,
        ._update = field_update,
        ._set_null = field_set_null,
        ._is_null = field_is_null,
        ._close = field_close
    };
    field_rebase(result, self);
    return result;
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

static inline void tuplet_close(tuplet_t *self)
{
    //assert (self != NULL);
    //free (self);



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

static inline bool field_next(tuplet_field_t *field)
{
    assert (field);
    assert (field->tuplet);
    assert (field->tuplet->fragment);

    enum tuplet_format format = field->tuplet->fragment->format;
    REQUIRE_VALID_TUPLET_FORMAT(format);

    const attr_id_t next_attr_id = field->attr_id + 1;
    if (next_attr_id < field->tuplet->fragment->schema->attr->num_elements) {
        //field_rebase(tuplet_field, tuplet_field->tuplet, next_attr_id);
        field_movebase(field, field->tuplet);
        return true;
    } else {
        bool valid_tuplet = gs_tuplet_next(field->tuplet);
        if (valid_tuplet) {
            field_rebase(field, field->tuplet);
            return true;
        } else {
            //gs_tuplet_field_close(tuplet_field);
            return false;
        }
    }
}

static inline bool field_seek(tuplet_field_t *field, attr_id_t attr_id)
{
    bool result = true;
    while(attr_id-- && result)
        result &= field_next(field);
    return result;
}

static inline const void *field_read(tuplet_field_t *field)
{
    assert (field);

   /* // DEBUG:
    if (tuplet_field->attr_id == 0) {
        printf("read tuplet (%u %p) tuplet_field attr0 (%llu %p): '%llu'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u64*) tuplet_field->attr_value_ptr);
    } else if (tuplet_field->attr_id == 1) {
        printf("read tuplet (%u %p) tuplet_field attr1 (%llu %p): '%d'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u32*) tuplet_field->attr_value_ptr);
    } else if (tuplet_field->attr_id == 2) {
        printf("read tuplet (%u %p) tuplet_field attr2 (%llu %p): '%d'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u16*) tuplet_field->attr_value_ptr);
    } else if (tuplet_field->attr_id == 3) {
        printf("read tuplet (%u %p) tuplet_field attr3 (%llu %p): '%d'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u16*) tuplet_field->attr_value_ptr);
    }
    // --
*/
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
/*
// DEBUG:
if (tuplet_field->attr_id == 0) {
    printf("wrote tuplet (%u %p) tuplet_field attr0 (%llu %p): '%llu'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u64*) tuplet_field->attr_value_ptr);
} else if (tuplet_field->attr_id == 1) {
    printf("wrote tuplet (%u %p) tuplet_field attr1 (%llu %p): '%d'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u32*) tuplet_field->attr_value_ptr);
} else if (tuplet_field->attr_id == 2) {
    printf("wrote tuplet (%u %p) tuplet_field attr2 (%llu %p): '%d'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u16*) tuplet_field->attr_value_ptr);
} else if (tuplet_field->attr_id == 3) {
    printf("wrote tuplet (%u %p) tuplet_field attr3 (%llu %p): '%d'\n", tuplet_field->tuplet->tuplet_id, tuplet_field->tuplet, tuplet_field->attr_id, tuplet_field->attr_value_ptr, *(u16*) tuplet_field->attr_value_ptr);
}*/
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

static inline void field_close(tuplet_field_t *field)
{
    assert (field);
    free (field);
}