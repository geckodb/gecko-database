#include <schema.h>
#include <attr.h>
#include <field.h>

attr_t *gs_schema_attr_by_id(const schema_t *frag, attr_id_t attr_id)
{
    return ((attr_t *)vector_at(frag->attr, attr_id));
}

schema_t *gs_schema_create()
{
    schema_t *result = malloc(sizeof(schema_t));
    result->attr = vector_create(sizeof(attr_t), 100);
    return result;
}

void gs_schema_free(schema_t *schema)
{
    assert (schema);
    vector_free(schema->attr);
    free (schema);
}
