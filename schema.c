#include <schema.h>
#include <attr.h>
#include <tuplet_field.h>

const struct attr_t *gs_schema_attr_by_id(const schema_t *schema, attr_id_t attr_id)
{
    assert (attr_id < schema->attr->num_elements);
    return ((struct attr_t *)vector_at(schema->attr, attr_id));
}

const struct attr_t *gs_schema_attr_by_name(const schema_t *schema, const char *name)
{
    assert (schema);
    for (size_t i = 0; i < schema->attr->num_elements; i++) {
        attr_t *attr = vector_at(schema->attr, i);
        if (!strcmp(attr->name, name)) {
            return attr;
        }
    }
    return NULL;
}

schema_t *gs_schema_create(const char *table_name)
{
    assert (table_name);
    schema_t *result = REQUIRE_MALLOC(sizeof(schema_t));
    result->attr = vector_create(sizeof(attr_t), 100);
    result->frag_name = strdup(table_name);
    return result;
}

schema_t *gs_schema_subset(schema_t *super, const attr_id_t *indicies, size_t nindicies)
{
    REQUIRE_NONNULL(super);
    REQUIRE_NONNULL(indicies);

    panic_if(nindicies > super->attr->num_elements, BADINTERNAL,
             "selected indices of super schema illegal.");

    schema_t *schema = gs_schema_create(super->frag_name);
    while (nindicies--) {
        attr_id_t i = *(indicies++);
        const attr_t *attr = gs_schema_attr_by_id(super, i);
        gs_attr_cpy(attr, schema);
    }
    return schema;
}

void gs_schema_free(schema_t *schema)
{
    assert (schema);
    vector_free(schema->attr);
    free (schema->frag_name);
    free (schema);
}

schema_t *gs_schema_cpy(const schema_t *schema)
{
    assert (schema);
    schema_t *cpy = REQUIRE_MALLOC(sizeof(schema_t));
    cpy->attr = vector_deep_cpy(schema->attr);
    cpy->frag_name = strdup(schema->frag_name);
    return cpy;
}

size_t gs_schema_num_attributes(const schema_t *schema)
{
    assert (schema);
    return schema->attr->num_elements;
}

const attr_id_t *gs_schema_attributes(const schema_t *schema)
{
    assert (schema);
    return schema->attr->data;
}

enum field_type gs_schema_attr_type(schema_t *schema, attr_id_t id)
{
    assert(schema);
    const attr_t *attr = gs_schema_attr_by_id(schema, id);
    return attr->type;
}