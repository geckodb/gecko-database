#include <schema.h>
#include <attr.h>
#include <field.h>

attr_t *gs_schema_attr_by_id(schema_t *frag, attr_id_t attr_id)
{
    return ((attr_t *)vector_at(frag->attr, attr_id));
}

schema_t *gs_schema_create()
{
    schema_t *result = malloc(sizeof(schema_t));
    result->attr = vector_create(sizeof(attr_t), 100);
    return result;
}
