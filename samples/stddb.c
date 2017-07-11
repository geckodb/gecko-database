
#include <stdio.h>
#include <stdlib.h>
#include <frag.h>
#include <field.h>

int main() {

   // const size_t NUM_TUPLETS = 4;

    schema_t *schema = gs_schema_create();

    gs_attr_create_uint64("My Attribute", schema);
    gs_attr_create_string("My Attribute 2", 42, schema);
    gs_attr_create_bool("My Attribute Bool", schema);

    fragment_t *fragment;
    tuplet_t *tuplet;

    fragment = gs_fragment_alloc(schema, 1, TF_NSM);
    tuplet   = gs_fragment_insert(fragment, 4);

    struct field_t *field = gs_field_open(tuplet);

    //UINT64 int_value;
    //CHAR *str_value;
    //BOOL bol_value;


/*    int_value = 1;                cont = gs_insert_uint64(data, schema, attr_1, &int_value);
    str_value = "Hello\n";        cont = gs_insert_string(cont, schema, attr_2, str_value);
    bol_value = true;             cont = gs_insert_bool  (cont, schema, attr_3, &bol_value);

    int_value = 2;                cont = gs_insert_uint64(data, schema, attr_1, &int_value);
    str_value = "World\n";        cont = gs_insert_string(cont, schema, attr_2, str_value);
    bol_value = true;             cont = gs_insert_bool  (cont, schema, attr_3, &bol_value);

    int_value = 3;                cont = gs_insert_uint64(data, schema, attr_1, &int_value);
    str_value = "Hi\n";           cont = gs_insert_string(cont, schema, attr_2, str_value);
    bol_value = true;             cont = gs_insert_bool  (cont, schema, attr_3, &bol_value);

    int_value = 42;               cont = gs_insert_uint64(data, schema, attr_1, &int_value);
    str_value = "There\n";        cont = gs_insert_string(cont, schema, attr_2, str_value);
    bol_value = true;             cont = gs_insert_bool  (cont, schema, attr_3, &bol_value);*/

    gs_field_close(field);
    gs_tuplet_close(tuplet);


    return EXIT_SUCCESS;
}

