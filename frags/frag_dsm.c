#include <frags/frag_dsm.h>
#include <operators/scan.h>
#include <field.h>

void this_fragment_dsm_dipose(frag_t *self);
tuplet_t *this_fragment_dsm_open(frag_t *self);

field_t *this_tuplet_open(tuplet_t *self, attr_id_t attr_id);
void this_tuplet_update(tuplet_t *self, const void *data);
void this_tuplet_set_null(tuplet_t *self);
void this_tuplet_delete(tuplet_t *self);

frag_t *gs_fragment_dsm_alloc(schema_t *schema, size_t tuplet_capacity)
{
    frag_t *fragment = malloc(sizeof(frag_t));
    *fragment = (frag_t) {
            .schema = schema,
            .format = TF_DSM,
            .ntuplets = 0,
            .ncapacity = tuplet_capacity,
            .tuplet_data = malloc (gs_tuplet_size_by_schema(schema) * tuplet_capacity),
            .tuplet_size = gs_tuplet_size_by_schema(schema),
            ._dispose = this_fragment_dsm_dipose,
            ._scan = scan_mediator,
            ._open = this_fragment_dsm_open
            // TODO: link to tuplet ops
    };
    return fragment;
}

void this_fragment_dsm_dipose(frag_t *self)
{
    free (self->tuplet_data);
    free (self);
}

tuplet_t *this_fragment_dsm_open(frag_t *self)
{
    return NULL;
}

void this_tuplet_next(tuplet_t *self)
{

}

field_t *this_tuplet_open(tuplet_t *self, attr_id_t attr_id)
{
    return NULL;
}

void this_tuplet_update(tuplet_t *self, const void *data)
{

}

void this_tuplet_set_null(tuplet_t *self)
{

}

void this_tuplet_delete(tuplet_t *self)
{

}