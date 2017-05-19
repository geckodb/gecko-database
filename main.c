
#include <containers/vector.h>
#include <error.h>
#include <storage/schema.h>
#include <storage/attribute.h>
#include <async.h>
#include <assert.h>

void test_promise_on_main_thread(const char *string, enum mdb_async_eval_policy policy);

void *sample_promise_print(enum mdb_async_promise_return *return_value, const void *capture) {
    assert (capture != NULL);
    const char *message = (char *) capture;
    printf("Printed from promise: '%s'\n", message);

    if (strcmp(message, "Hello World") == 0) {
        printf(">> Resolved!\n");
        *return_value = APR_RESOLVED;
        int *result = malloc(sizeof(int));
        *result = 42;
        return result;
    } else {
        printf(">> Rejected!\n");
        thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
        *return_value = APR_REJECTED;
        return NULL;
    }
}

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
        printf("FOUND 'A3'?: %d, name = %s\n\n", (cursor != NULL), (cursor != NULL ? cursor->name : "n/a"));

        mdb_schema_free(s);

        test_promise_on_main_thread("Hello World", AEP_LAZY);
        test_promise_on_main_thread("Bonjour World", AEP_LAZY);
    }


    return 0;

}

void test_promise_on_main_thread(const char *string, enum mdb_async_eval_policy policy) {

    mdb_async_future *future = mdb_async_future_alloc(string, sample_promise_print, policy);
    if (future == NULL) {
      error_print();
    } else {
        enum mdb_async_promise_return return_type;
        const void *result = mdb_async_future_resolve(&return_type, future);
        printf("RETURNED FROM PROMISE: %d\n", (result == NULL ? -1 : *(int *)result));
        mdb_async_future_free(future);
    }

}