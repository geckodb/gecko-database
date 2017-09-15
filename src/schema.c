// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <schema.h>
#include <attr.h>
#include <tuplet_field.h>
#include <debug.h>

const struct attr_t *schema_attr_by_id(const schema_t *schema, attr_id_t attr_id)
{
    assert (attr_id < schema->attr->num_elements);
    return ((struct attr_t *) vec_at(schema->attr, attr_id));
}

const struct attr_t *schema_attr_by_name(const schema_t *schema, const char *name)
{
    assert (schema);
    for (size_t i = 0; i < schema->attr->num_elements; i++) {
        attr_t *attr = vec_at(schema->attr, i);
        if (!strcmp(attr->name, name)) {
            return attr;
        }
    }
    return NULL;
}

schema_t *schema_new(const char *table_name)
{
    assert (table_name);
    schema_t *result = REQUIRE_MALLOC(sizeof(schema_t));
    result->attr = vec_new(sizeof(attr_t), 100);
    result->frag_name = strdup(table_name);
    return result;
}

schema_t *schema_subset(schema_t *super, const attr_id_t *indicies, size_t nindicies)
{
    REQUIRE_NONNULL(super);
    REQUIRE_NONNULL(indicies);

    panic_if(nindicies > super->attr->num_elements, BADINTERNAL,
             "selected indices of super schema illegal.");

    schema_t *schema = schema_new(super->frag_name);
    while (nindicies--) {
        attr_id_t i = *(indicies++);
        const attr_t *attr = schema_attr_by_id(super, i);
        attr_cpy(attr, schema);
    }
    return schema;
}

void schema_delete(schema_t *schema)
{
    assert (schema);
    vec_free(schema->attr);
    free (schema->frag_name);
    free (schema);
}

schema_t *schema_cpy(const schema_t *schema)
{
    assert (schema);
    schema_t *cpy = REQUIRE_MALLOC(sizeof(schema_t));
    cpy->attr = vec_cpy_deep(schema->attr);
    cpy->frag_name = strdup(schema->frag_name);
    return cpy;
}

size_t schema_num_attributes(const schema_t *schema)
{
    assert (schema);
    return schema->attr->num_elements;
}

const attr_id_t *schema_attributes(const schema_t *schema)
{
    assert (schema);
    return schema->attr->data;
}

enum field_type schema_attr_type(schema_t *schema, attr_id_t id)
{
    assert(schema);
    const attr_t *attr = schema_attr_by_id(schema, id);
    return attr->type;
}

void schema_print(FILE *file, schema_t *schema)
{
    schema_t *print_schema = schema_new("ad hoc info");
    attr_create_attrid("attr_id", print_schema);
    attr_create_strptr("name", print_schema);
    attr_create_strptr("type", print_schema);
    attr_create_uint64("rep", print_schema);
    attr_create_bool("primary", print_schema);
    attr_create_bool("foreign", print_schema);
    attr_create_bool("nullable", print_schema);
    attr_create_bool("autoinc", print_schema);
    attr_create_bool("unique", print_schema);
    frag_t *frag = frag_new(print_schema, 1, FIT_HOST_NSM_VM);
    size_t num_attr = schema_num_attributes(schema);
    tuplet_t tuplet;
    frag_insert(&tuplet, frag, num_attr);

    do {
        tuplet_field_t field;
        tuplet_field_open(&field, &tuplet);
        for (attr_id_t i = 0; i < num_attr; i++) {
            const attr_t *attr = schema_attr_by_id(schema, i);
            tuplet_field_write(&field, &i, true);
            tuplet_field_write(&field, attr->name, true);
            tuplet_field_write(&field, field_type_str(attr->type), true);
            tuplet_field_write(&field, &attr->type_rep, true);
            tuplet_field_write_eval(&field, (attr->flags.primary == 1), true);
            tuplet_field_write_eval(&field, (attr->flags.foreign == 1), true);
            tuplet_field_write_eval(&field, (attr->flags.nullable == 1), true);
            tuplet_field_write_eval(&field, (attr->flags.autoinc == 1), true);
            tuplet_field_write_eval(&field, (attr->flags.unique == 1), true);
        }
    } while (tuplet_next(&tuplet));

    frag_print(file, frag, 0, INT_MAX);

    frag_delete(frag);
    schema_delete(print_schema);
}