
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

void run_with_hash_function(hash_function_t *hash_function, const char *hash_function_name)
{
    const size_t NUM_ELEMENTS = 40000000 * 2;
    const size_t NUM_SLOTS = NUM_ELEMENTS * 2;

    dictionary_t *dict = linear_hash_table_create(hash_function,
                                                  sizeof(size_t), sizeof(test_data_t), NUM_SLOTS, GROW_FACTOR);

    clock_t start, stop;

    /*******************************************************************************************************************
     * MEASURE PUT CALL TIME
     ******************************************************************************************************************/
    start = clock();
    /*for (int i = 0; i < NUM_ELEMENTS; i++) {
        size_t key = rand();
        test_data_t value = {
                .a = 23,
                .b = 42,
                .c = 114
        };
        dictionary_put(dict, &key, &value);
    }*/
    stop = clock();
    dictionary_clear(dict);
    double put_call_elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    /*******************************************************************************************************************
     * MEASURE PUTS CALL TIME
     ******************************************************************************************************************/
    size_t *keys = malloc (sizeof(size_t) * NUM_ELEMENTS);
    test_data_t *values = malloc (sizeof(test_data_t) * NUM_ELEMENTS);

    linear_hash_reset_counters(dict);
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

    printf("%s;%zu;%zu;%0.2f;%0.2f;%0.2f;%zu;%zu;%zu;%zu;%0.4f;%0.4f;%zu;%zu\n",
           hash_function_name,
           info.num_slots_inuse, info.num_slots_free, info.load_factor,
           info.overhead_size / 1024.0 / 1024.0, info.user_data_size / 1024.0 / 1024.0,
           info.counters.num_locks, info.counters.num_collisions, info.counters.num_rebuilds, info.counters.num_put_calls,
           put_call_elapsed, puts_call_elapsed, info.counters.num_put_slotsearch, info.counters.num_updates);

    linear_hash_table_free(dict);
}

int main(void)
{
    srand(time(NULL));

    printf("hash_fn;slots_inuse;slots_free;load_factor;footprint_overhead_mib;footprint_user_mib;num_locks;num_collisions;"
                   "num_rebuilds;num_putcalls;elapsed_put_calls_ms;elapsed_puts_calls_ms;num_put_slotsearch;"
                   "num_updates\n");

    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen}, "jenkins");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_identity_size_t}, "identity");
    //run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_additive}, "additive");
    //run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_xor}, "xor");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_rot}, "rot");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_bernstein}, "bernstein");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_bernstein2}, "bernstein2");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_sax}, "sax");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_fnv}, "fnv");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_oat}, "oat");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jsw}, "jsw");
    run_with_hash_function(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_elf}, "elf");

    return 0;

}
