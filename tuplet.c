#include <tuplet.h>
#include <tuplet_field.h>
#include <schema.h>

void gs_tuplet_open(tuplet_t *dst, struct frag_t *frag, tuplet_id_t tuplet_id)
{
    if (frag->ntuplets > 0) {
        REQUIRE_NONNULL(frag);
        REQUIRE_NONNULL(frag->_open);
        frag->_open(dst, frag, tuplet_id);
    }
}

bool gs_tuplet_next(tuplet_t *tuplet)
{
    REQUIRE_NONNULL(tuplet);
    REQUIRE_NONNULL(tuplet->_next);
    return tuplet->_next(tuplet);
}

void gs_tuplet_rewind(tuplet_t *tuplet)
{
    REQUIRE_NONNULL(tuplet);
    frag_t *frag = tuplet->fragment;
    gs_tuplet_open(tuplet, frag, 0);
}

void gs_tuplet_set_null(tuplet_t *tuplet)
{
    REQUIRE_NONNULL(tuplet);
    REQUIRE_NONNULL(tuplet->_set_null);
    tuplet->_set_null(tuplet);
}

bool gs_tuplet_is_null(tuplet_t *tuplet)
{
    REQUIRE_NONNULL(tuplet);
    REQUIRE_NONNULL(tuplet->_is_null);
    return (tuplet->_is_null(tuplet));
}

size_t gs_tuplet_size(tuplet_t *tuplet)
{
    REQUIRE_NONNULL(tuplet);
    REQUIRE_NONNULL(tuplet->fragment);
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

enum field_type gs_tuplet_get_field_type(const tuplet_t *tuplet, attr_id_t id)
{
    assert(tuplet);
    return gs_fragment_get_field_type(tuplet->fragment, id);
}
