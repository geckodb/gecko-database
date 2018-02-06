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

#include <gs_schema.h>
#include <gs_attr.h>
#include <gs_tuplet_field.h>
#include <gecko-commons/gs_debug.h>


// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

const struct gs_attr_t *gs_schema_attr_by_id(const gs_schema_t *schema, gs_attr_id_t attr_id)
{
    assert (attr_id < schema->attr->num_elements);
    return ((struct gs_attr_t *) gs_vec_at(schema->attr, attr_id));
}

const struct gs_attr_t *gs_schema_attr_by_name(const gs_schema_t *schema, const char *name)
{
    assert (schema);
    for (size_t i = 0; i < schema->attr->num_elements; i++) {
        gs_attr_t *attr = gs_vec_at(schema->attr, i);
        if (!strcmp(attr->name, name)) {
            return attr;
        }
    }
    return NULL;
}

gs_schema_t *gs_schema_new(const char *table_name)
{
    assert (table_name);
    gs_schema_t *result = GS_REQUIRE_MALLOC(sizeof(gs_schema_t));
    result->attr = gs_vec_new(sizeof(gs_attr_t), 100);
    result->frag_name = strdup(table_name);
    return result;
}

gs_schema_t *gs_schema_subset(gs_schema_t *super, const gs_attr_id_t *indicies, size_t nindicies)
{
    GS_REQUIRE_NONNULL(super);
    GS_REQUIRE_NONNULL(indicies);

    panic_if(nindicies > super->attr->num_elements, BADINTERNAL,
             "selected indices of super schema illegal.");

    gs_schema_t *schema = gs_schema_new(super->frag_name);
    while (nindicies--) {
        gs_attr_id_t i = *(indicies++);
        const gs_attr_t *attr = gs_schema_attr_by_id(super, i);
        gs_attr_cpy(attr, schema);
    }
    return schema;
}

void gs_schema_delete(gs_schema_t *schema)
{
    assert (schema);
    gs_vec_free(schema->attr);
    free (schema->frag_name);
    free (schema);
}

gs_schema_t *gs_schema_cpy(const gs_schema_t *schema)
{
    assert (schema);
    gs_schema_t *cpy = GS_REQUIRE_MALLOC(sizeof(gs_schema_t));
    cpy->attr = gs_vec_cpy_deep(schema->attr);
    cpy->frag_name = strdup(schema->frag_name);
    return cpy;
}

size_t gs_schema_num_attributes(const gs_schema_t *schema)
{
    assert (schema);
    return schema->attr->num_elements;
}

const gs_attr_id_t *gs_schema_attributes(const gs_schema_t *schema)
{
    assert (schema);
    return schema->attr->data;
}

enum gs_field_type_e gs_schema_attr_type(gs_schema_t *schema, gs_attr_id_t id)
{
    assert(schema);
    const gs_attr_t *attr = gs_schema_attr_by_id(schema, id);
    return attr->type;
}

void gs_schema_print(FILE *file, gs_schema_t *schema)
{
    gs_schema_t *print_schema = gs_schema_new("ad hoc info");
    attr_create_attrid("attr_id", print_schema);
    attr_create_strptr("name", print_schema);
    attr_create_strptr("type", print_schema);
    attr_create_uint64("rep", print_schema);
    attr_create_bool("primary", print_schema);
    attr_create_bool("foreign", print_schema);
    attr_create_bool("nullable", print_schema);
    attr_create_bool("autoinc", print_schema);
    attr_create_bool("unique", print_schema);
    gs_frag_t *frag = gs_frag_new(print_schema, 1, FIT_HOST_NSM_VM);
    size_t num_attr = gs_schema_num_attributes(schema);
    gs_tuplet_t tuplet;
    gs_frag_insert(&tuplet, frag, num_attr);

    do {
        gs_tuplet_field_t field;
        gs_tuplet_field_open(&field, &tuplet);
        for (gs_attr_id_t i = 0; i < num_attr; i++) {
            const gs_attr_t *attr = gs_schema_attr_by_id(schema, i);
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

    gs_frag_delete(frag);
    gs_schema_delete(print_schema);
}