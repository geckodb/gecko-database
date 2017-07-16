#include <schema.h>
#include <attr.h>
#include <field.h>

attr_t *gs_schema_attr_by_id(const schema_t *schema, attr_id_t attr_id)
{
    assert (attr_id < schema->attr->num_elements);
    return ((attr_t *)vector_at(schema->attr, attr_id));
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

schema_t *gs_schema_cpy(schema_t *schema)
{
    assert (schema);
    schema_t *cpy = require_good_malloc(sizeof(schema_t));
    cpy->attr = vector_cpy(schema->attr);
    return cpy;
}

size_t gs_schema_num_attributes(schema_t *schema)
{
    assert (schema);
    return schema->attr->num_elements;
}

enum field_type gs_schema_attr_type(schema_t *schema, attr_id_t id)
{
    assert(schema);
    attr_t *attr = gs_schema_attr_by_id(schema, id);
    return attr->type;
}