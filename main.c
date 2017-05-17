
#include <containers/vector.h>
#include <error.h>
#include <storage/schema.h>
#include <storage/attribute.h>

int main(void)
{
    for (int i = 0; i < 1000; i++) {
        mdb_vector *p;
        if ((p = mdb_vector_alloc(sizeof(char), 3)) == NULL) {
            error_print();
        };
        if (!mdb_vector_add(p, 16, "Eine Kleine Maus")) {
            error_print();
        }

        size_t size = p->sizeof_element;
        size_t num = p->num_elements;
        const void *data = mdb_vector_get(p);

        for (int i = 0; i < num; i++) {
            printf("%d: %c\n", i, *((char *) data + (i * size)));
        }

        mdb_schema *s = mdb_schema_alloc(2);
        mdb_attribute *a1 = mdb_attribute_alloc(TYPE_INTEGER, 1, "A1", 0);
        mdb_schema_add(s, a1);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        mdb_attribute_free(a1);

        mdb_attribute *a2 = mdb_attribute_alloc(TYPE_FIX_STRING, 200, "A2", 0);
        mdb_schema_add(s, a2);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        mdb_attribute_free(a2);

        mdb_attribute *a3 = mdb_attribute_alloc(TYPE_BYTE, 1, "A3", 0);
        mdb_schema_add(s, a3);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        mdb_attribute_free(a3);

        const mdb_attribute *cursor = mdb_schema_get_attr_by_name(s, "A0");
        printf("FOUND 'A3'?: %d, name = %s", (cursor != NULL), (cursor != NULL ? cursor->name : "n/a"));

        mdb_schema_free(s);
    }


    return 0;

}