
#include <containers/vector.h>
#include <error.h>
#include <storage/schema.h>
#include <storage/attribute.h>
#include <async.h>
#include <assert.h>

void test_promise_on_main_thread(const char *string, future_eval_policy policy);

void *sample_promise_print(promise_result *return_value, const void *capture) {
    assert (capture != NULL);
    const char *message = (char *) capture;
    printf("Printed from promise: '%s'\n", message);

    if (strcmp(message, "Hello World") == 0) {
        printf(">> Resolved!\n");
        *return_value = resolved;
        int *result = malloc(sizeof(int));
        *result = 42;
        return result;
    } else {
        printf(">> Rejected!\n");
        thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
        *return_value = rejected;
        return NULL;
    }
}

int main(void)
{
    for (int i = 0; i < 1000; i++) {
        vector_t *p;
        if ((p = vector_create(sizeof(char), 3)) == NULL) {
            error_print();
        };
        if (!vector_add(p, 16, "Eine Kleine Maus")) {
            error_print();
        }

        size_t size = p->sizeof_element;
        size_t num = p->num_elements;
        const void *data = vector_get(p);

        for (int i = 0; i < num; i++) {
            printf("%d: %c\n", i, *((char *) data + (i * size)));
        }

        schema_t *s = schema_create(2);
        attribute_t *a1 = attribute_create(type_int, 1, "A1", 0);
        schema_add(s, a1);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        attribute_free(a1);

        attribute_t *a2 = attribute_create(type_fix_string, 200, "A2", 0);
        schema_add(s, a2);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        attribute_free(a2);

        attribute_t *a3 = attribute_create(type_byte, 1, "A3", 0);
        schema_add(s, a3);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        attribute_free(a3);

        const attribute_t *cursor = schema_get_by_name(s, "A0");
        printf("FOUND 'A3'?: %d, name = %s\n\n", (cursor != NULL), (cursor != NULL ? cursor->name : "n/a"));

        schema_free(s);

        test_promise_on_main_thread("Hello World", future_lazy);
        test_promise_on_main_thread("Bonjour World", future_lazy);
    }


    return 0;

}

void test_promise_on_main_thread(const char *string, future_eval_policy policy) {

    future_t future = future_create(string, sample_promise_print, policy);
    if (future == NULL) {
      error_print();
    } else {
        promise_result return_type;
        const void *result = future_resolve(&return_type, future);
        printf("RETURNED FROM PROMISE: %d\n", (result == NULL ? -1 : *(int *)result));
    }

}