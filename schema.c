#include <schema.h>
#include <attr.h>
#include <field.h>

const struct attr_t *gs_schema_attr_by_id(const schema_t *schema, attr_id_t attr_id)
{
    assert (attr_id < schema->attr->num_elements);
    return ((struct attr_t *)vector_at(schema->attr, attr_id));
}

schema_t *gs_schema_create(const char *table_name)
{
    assert (table_name);
    schema_t *result = malloc(sizeof(schema_t));
    result->attr = vector_create(sizeof(attr_t), 100);
    result->table_name = strdup(table_name);
    return result;
}

schema_t *gs_schema_subset(schema_t *super, vector_t /*of attr_id_t */ *indices)
{
    /*panic_if(indices->num_elements > super->attr->num_elements, BADINTERNAL,
             "selected indices of super schema illegal.");
    schema_t *result = gs_schema_create();
    const attr_id_t *indices_data = (const attr_id_t *) vector_get(indices);
    for (size_t i = 0; i < indices->num_elements; i++) {
        const struct attr_t *attr = gs_schema_attr_by_id(super, indices_data[i]);
        //gs_attr_cpy
    }*/ // TODO
return NULL;

}

void gs_schema_free(schema_t *schema)
{
    assert (schema);
    vector_free(schema->attr);
    free (schema->table_name);
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
    const attr_t *attr = gs_schema_attr_by_id(schema, id);
    return attr->type;
}