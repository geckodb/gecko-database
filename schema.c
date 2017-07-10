#include <schema.h>

attr_t *gs_schema_attr_by_id(schema_t *frag, ATTR_ID attr_id)
{
    return ((attr_t *)vector_at(frag->attr, attr_id));
}