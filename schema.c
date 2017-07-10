#include <schema.h>

ATTR *gs_schema_attr_by_id(SCHEMA *frag, ATTR_ID attr_id)
{
    return ((ATTR *)vector_at(frag->attr, attr_id));
}