#include <schema.h>
#include <attr.h>
#include <tuplet_field.h>
#include <debug.h>

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

void gs_schema_print(FILE *file, schema_t *schema)
{
    schema_t *print_schema = gs_schema_create("ad hoc info");
    gs_attr_create_attrid("attr_id", print_schema);
    gs_attr_create_strptr("name", print_schema);
    gs_attr_create_strptr("type", print_schema);
    gs_attr_create_uint64("rep", print_schema);
    gs_attr_create_bool("primary", print_schema);
    gs_attr_create_bool("foreign", print_schema);
    gs_attr_create_bool("nullable", print_schema);
    gs_attr_create_bool("autoinc", print_schema);
    gs_attr_create_bool("unique", print_schema);
    frag_t *frag = gs_fragment_alloc(print_schema, 1, FIT_HOST_NSM_VM);
    size_t num_attr = gs_schema_num_attributes(schema);
    tuplet_t tuplet;
    gs_frag_insert(&tuplet, frag, num_attr);

    do {
        tuplet_field_t field;
        gs_tuplet_field_open(&field, &tuplet);
        for (attr_id_t i = 0; i < num_attr; i++) {
            const attr_t *attr = gs_schema_attr_by_id(schema, i);
            gs_tuplet_field_write(&field, &i, true);
            gs_tuplet_field_write(&field, attr->name, true);
            gs_tuplet_field_write(&field, gs_field_type_str(attr->type), true);
            gs_tuplet_field_write(&field, &attr->type_rep, true);
            gs_tuplet_field_write_eval(&field, (attr->flags.primary == 1), true);
            gs_tuplet_field_write_eval(&field, (attr->flags.foreign == 1), true);
            gs_tuplet_field_write_eval(&field, (attr->flags.nullable == 1), true);
            gs_tuplet_field_write_eval(&field, (attr->flags.autoinc == 1), true);
            gs_tuplet_field_write_eval(&field, (attr->flags.unique == 1), true);
        }
    } while (gs_tuplet_next(&tuplet));

    gs_frag_print(file, frag, 0, INT_MAX);

    gs_frag_free(frag);
    gs_schema_free(print_schema);
}