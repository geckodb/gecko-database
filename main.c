
#include <containers/vector.h>
#include <error.h>
#include <storage/schema.h>

int main(void)
{
    for (int i = 0; i < 1000; i++) {
        struct vector *p;
        if ((p = vector_create(sizeof(char), 3)) == NULL) {
            error_print();
        };
        if (!vector_push_back(p, 16, "Eine Kleine Maus")) {
            error_print();
        }

        size_t size = vector_get_elements_size(p);
        size_t num = vector_get_num_elements(p);
        const void *data = vector_get_data(p);

        for (int i = 0; i < num; i++) {
            printf("%d: %c\n", i, *((char *) data + (i * size)));
        }

        struct schema *s = schema_create(5);
        struct attribute *a1 = attribute_create(DT_BOOLEAN, "A1");
        schema_set_attribute(s, 0, a1);
        printf("NUM attributes: %zu\n", schema_get_num_of_attributes(s));
        attribute_delete(a1);

        schema_delete(s);
    }


    return 0;

}