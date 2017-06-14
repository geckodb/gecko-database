
#include <stddef.h>
#include <stdlib.h>
#include <defs.h>
#include <time.h>
#include <containers/dictionary.h>
#include <containers/dictionaries/linear_hash_table.h>

typedef struct test_data_t {
    size_t a, b, c;
} test_data_t;

void print_to_stdout(void *counter, const void *key, const void *value)
{
    test_data_t data = *(test_data_t *) value;
    size_t *i = (size_t *) counter;
    printf("idx=%zu: key=%zu, value=(a=%zu, b=%zu, c=%zu)\n", *i, *(size_t *) key, data.a, data.b, data.c);
    *i = *i + 1;
}

int main(void)
{
    srand(time(NULL));

    const size_t NUM_SLOTS = 15000;
    const size_t NUM_ELEMENTS = 10000000;

    dictionary_t *dict = linear_hash_table_create(hash_code_size_t, hash_fn_mod, sizeof(size_t), sizeof(test_data_t),
                                                  NUM_SLOTS, GROW_FACTOR);

    clock_t start, stop;

    /*******************************************************************************************************************
     * MEASURE PUT CALL TIME
     ******************************************************************************************************************/
    start = clock();
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        size_t key = rand();
        test_data_t value = {
            .a = 23,
            .b = 42,
            .c = 114
        };
        dictionary_put(dict, &key, &value);
    }
    stop = clock();
    dictionary_clear(dict);
    double put_call_elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    /*******************************************************************************************************************
     * MEASURE PUTS CALL TIME
     ******************************************************************************************************************/
    size_t *keys = malloc (sizeof(size_t) * NUM_ELEMENTS);
    test_data_t *values = malloc (sizeof(test_data_t) * NUM_ELEMENTS);
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        keys[i] = rand();
        values[i] = (test_data_t) {
                .a = 23,
                .b = 42,
                .c = 114
        };
    }

    start = clock();
    dictionary_puts(dict, NUM_ELEMENTS, keys, values);
    stop = clock();
    free (keys);
    free (values);

    double puts_call_elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;



    linear_hash_table_info_t info;
    linear_hash_table_info(dict, &info);

    printf("slots_inuse;slots_free;load_factor;footprint_overhead_mib;footprint_user_mib;num_locks;num_collisions;"
           "num_rebuilds;num_putcalls;elapsed_put_calls_ms;elapsed_puts_calls_ms\n");

    printf("%zu;%zu;%0.2f;%0.2f;%0.2f;%zu;%zu;%zu;%zu;%0.4f;%0.4f\n",
            info.num_slots_inuse, info.num_slots_free, info.load_factor,
            info.overhead_size / 1024.0 / 1024.0, info.user_data_size / 1024.0 / 1024.0,
            info.counters.num_locks, info.counters.num_collisions, info.counters.num_rebuilds, info.counters.num_put_calls,
            put_call_elapsed, puts_call_elapsed);

    linear_hash_table_free(dict);


    return 0;

}
