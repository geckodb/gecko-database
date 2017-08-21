#include <tuplet.h>
#include <tuplet_field.h>
#include <schema.h>

tuplet_t *gs_tuplet_open(struct frag_t *frag)
{
    if (frag->ntuplets > 0) {
        require_non_null(frag);
        require_non_null(frag->_open);
        tuplet_t *result = frag->_open(frag);
        panic_if((result == NULL), UNEXPECTED, "frag_t::open return NULL");
        return result;
    } else return NULL;
}

bool gs_tuplet_next(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_next);
    return tuplet->_next(tuplet);
}

tuplet_t *gs_tuplet_rewind(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_close);
    frag_t *frag = tuplet->fragment;
    tuplet->_close(tuplet);
    return gs_tuplet_open(frag);
}

void gs_tuplet_set_null(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_set_null);
    tuplet->_set_null(tuplet);
}

bool gs_tuplet_is_null(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_is_null);
    return (tuplet->_is_null(tuplet));
}

void gs_tuplet_close(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->_close);
    tuplet->_close(tuplet);
}

size_t gs_tuplet_size(tuplet_t *tuplet)
{
    require_non_null(tuplet);
    require_non_null(tuplet->fragment);
    return tuplet->fragment->tuplet_size;
}

void *gs_update(void *dst, schema_t *frag, attr_id_t attr_id, void *src)
{
    panic(NOTIMPLEMENTED, to_string(gs_update))
    return NULL;
}

size_t gs_tuplet_size_by_schema(const schema_t *schema)
{
    size_t total = 0;
    for (size_t attr_idx = 0; attr_idx < schema->attr->num_elements; attr_idx++) {
        const struct attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
        total += gs_attr_total_size(attr);
    }
    return total;
}

enum field_type gs_tuplet_get_field_type(tuplet_t *tuplet, attr_id_t id)
{
    assert(tuplet);
    return gs_fragment_get_field_type(tuplet->fragment, id);
}
