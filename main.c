
#include <containers/vector.h>
#include <error.h>
#include <storage/schema.h>
#include <storage/attribute.h>
#include <async.h>
#include <assert.h>
#include <containers/list.h>
#include <storage/memory.h>
#include <containers/dict.h>
#include <containers/dictionaries/fixed_linear_hash_table.h>

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
        attribute_t *a1 = attribute_create(type_single_int16, 1, "A1", 0);
        schema_add(s, a1);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        attribute_free(a1);

        attribute_t *a2 = attribute_create(type_single_int16, 200, "A2", 0);
        schema_add(s, a2);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        attribute_free(a2);

        attribute_t *a3 = attribute_create(type_single_int16, 1, "A3", 0);
        schema_add(s, a3);
        printf("NUM attributes: %zu\n", s->attributes->num_elements);
        attribute_free(a3);

        const attribute_t *cursor = schema_get_by_name(s, "A0");
        printf("FOUND 'A3'?: %d, name = %s\n\n", (cursor != NULL), (cursor != NULL ? cursor->name : "n/a"));

        schema_free(s);

        test_promise_on_main_thread("Hello World", future_lazy);
        test_promise_on_main_thread("Bonjour World", future_eager);

        schema_t *fixed_schmea = schema_create(3);
        attribute_t *attr1 = attribute_create(type_single_int16, 1, "A1", 0);
        attribute_t *attr2 = attribute_create(type_single_int32, 1, "A2", 0);
        attribute_t *attr3 = attribute_create(type_single_int64, 1, "A3", 0);
        attribute_t *attr4 = attribute_create(type_multi_fixed, 20, "A4", 0);
        attribute_t *attr5 = attribute_create(type_multi_variable, 1, "A5", 0);
        attribute_t *attr6 = attribute_create(type_single_int64, 1, "A6", 0);

        schema_add(fixed_schmea, attr1);
        schema_add(fixed_schmea, attr2);
        schema_add(fixed_schmea, attr3);
        schema_add(fixed_schmea, attr4);
        schema_add(fixed_schmea, attr5);
        schema_add(fixed_schmea, attr6);


        attribute_free(attr1);
        attribute_free(attr2);
        attribute_free(attr3);
        schema_free(fixed_schmea);

        list_t *my_int_list = list_create(sizeof(uint64_t));
        uint64_t int_data = 23;
        bool op_result = false;
        assert (list_is_empty(my_int_list));
        op_result = list_push(my_int_list, &int_data);
        assert (op_result);
        const void *node_data = list_begin(my_int_list);
        assert (*((uint64_t *) node_data) == 23);
        const void *node_data2 = list_next(node_data);
        assert (node_data2 == NULL);
        assert (list_num_elements(my_int_list) == 1);
        op_result = list_remove(node_data);
        assert (op_result);
        assert (list_num_elements(my_int_list) == 0);
        op_result = list_push(my_int_list, &int_data);
        assert (op_result);
        int_data = 42;
        op_result = list_push(my_int_list, &int_data);
        assert (op_result);
        int_data = 1986;
        op_result = list_push(my_int_list, &int_data);
        assert (op_result);
        assert (list_num_elements(my_int_list) == 3);
        const void *data1 = list_begin(my_int_list);
        assert (data1 != NULL && *(uint64_t *) data1 == 23);
        const void *data2 = list_next(data1);
        assert (data2 != NULL && *(uint64_t *) data2 == 42);
        const void *data3 = list_next(data2);
        assert (data3 != NULL && *(uint64_t *) data3 == 1986);
        assert (list_next(data3) == NULL);
        list_remove(data1);
        assert (list_num_elements(my_int_list) == 2);

        const void *data4 = list_begin(my_int_list);
        assert (data4 != NULL && *(uint64_t *) data4 == 42);
        const void *data5 = list_next(data4);
        assert (data5 != NULL && *(uint64_t *) data5 == 1986);
        assert (list_next(data5) == NULL);

        list_remove(data5);
        assert (list_num_elements(my_int_list) == 1);
        const void *data6 = list_begin(my_int_list);
        assert (data6 != NULL && *(uint64_t *) data6 == 42);
        assert (list_next(data6) == NULL);

        int_data = 2010;
        op_result = list_push(my_int_list, &int_data);
        assert (op_result);
        int_data = 2017;
        op_result = list_push(my_int_list, &int_data);
        assert (op_result);

        const void *data7 = list_begin(my_int_list);
        assert (data7 != NULL && *(uint64_t *) data7 == 42);
        const void *data8 = list_next(data7);
        assert (data8 != NULL && *(uint64_t *) data8 == 2010);
        const void *data9 = list_next(data8);
        assert (data9 != NULL && *(uint64_t *) data9 == 2017);
        assert (list_next(data9) == NULL);

        op_result = list_remove(data7);
        assert (op_result);

        const void *data10 = list_begin(my_int_list);
        assert (data10 != NULL && *(uint64_t *) data10 == 2010);
        const void *data11 = list_next(data10);
        assert (data11 != NULL && *(uint64_t *) data11 == 2017);
        assert (list_next(data11) == NULL);

        list_free(my_int_list);

        printf("%zu\n", sizeof(page_t));
        fflush(stdout);
        buffer_manager_t *manager = buffer_manager_create();
        page_t *page = page_create(manager, 42, 1024 * 1024 /* 1 MiB */, page_flag_fixed, 10, 10);
        fid_t *frame_handle = frame_create(page, positioning_first_nomerge, 20 /* 40 B */);
        frame_create(page, positioning_first_nomerge, 2048 /* 2 KiB */);


        printf("****************** REMOVE ZONE CASE 1 **********************\n\n");
        printf("+---------------- THIS PAGE ----------------+       +---------------- THIS PAGE ----------------+\n");
        printf("|     v--------- frame -----|               |       |                                           |\n");
        printf("|  +-------+            +--------+          |       |  +-------+                                |\n");
        printf("|  | FRAME | - first -> | ZONE X | -> NULL  |   =>  |  | FRAME | - first ->   NULL              |\n");
        printf("|  +-------+            +--------+          |       |  +-------+                                |\n");
        printf("|     |-------- last -------^               |       |     |------- last -------^                |\n");
        printf("+-------------------------------------------+       +-------------------------------------------+\n");
        zone_t *zone_1 = zone_create(manager, page, frame_handle, positioning_first_nomerge);
        zone_memcpy(page, zone_1, "Hello Zone!", sizeof(char) * strlen("Hello Zone!"));
        page_dump(stdout, manager, page);

        // Remove one and only zone
        zone_remove(manager, page, zone_1);
        page_dump(stdout, manager, page);

        printf("****************** REMOVE ZONE CASE 2 **********************\n");
        zone_1 = zone_create(manager, page, frame_handle, positioning_first_nomerge);
        zone_memcpy(page, zone_1, "Hello Zone!", sizeof(char) * strlen("Hello Zone!"));
        zone_t *zone_2 = zone_create(manager, page, frame_handle, positioning_first_nomerge);
        zone_memcpy(page, zone_2, "Hello Zone 2!", sizeof(char) * strlen("Hello Zone 2!"));
        page_dump(stdout, manager, page);

        // Remove head only
        zone_remove(manager, page, zone_1);
        page_dump(stdout, manager, page);

        printf("****************** REMOVE ZONE CASE 3 **********************\n");
        zone_1 = zone_create(manager, page, frame_handle, positioning_first_nomerge);
        zone_memcpy(page, zone_1, "Hello Zone 1!", sizeof(char) * strlen("Hello Zone 1!"));
        page_dump(stdout, manager, page);

        // Remove tail only
        zone_remove(manager, page, zone_1);
        page_dump(stdout, manager, page);

        printf("****************** REMOVE ZONE CASE 4 **********************\n");
        zone_1 = zone_create(manager, page, frame_handle, positioning_first_nomerge);
        zone_memcpy(page, zone_1, "Hello Zone 1!", sizeof(char) * strlen("Hello Zone 1!"));
        zone_t *zone_3 = zone_create(manager, page, frame_handle, positioning_first_nomerge);
        zone_memcpy(page, zone_3, "Hello Zone 3!", sizeof(char) * strlen("Hello Zone 2!"));
        page_dump(stdout, manager, page);

        // Remove middle
        zone_remove(manager, page, zone_1);
        page_dump(stdout, manager, page);

        printf("\n\n\n\n");


        printf("\n");



     //   fid_t x;
    //    frame_delete(&x);
        exit(0);
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