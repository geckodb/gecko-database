
#include <stddef.h>
#include <stdlib.h>
#include <stdinc.h>
#include <time.h>
#include <containers/dict.h>
#include <containers/dicts/hash_table.h>

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
    const size_t NUM_ELEMENTS = 4000000 * 2;
    const size_t NUM_SLOTS = NUM_ELEMENTS * 2;

    dict_t *dict = hash_table_create(hash_function,
                                     sizeof(size_t), sizeof(test_data_t), NUM_SLOTS, GROW_FACTOR, 0.7f);

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
        dict_put(dict, &key, &value);
    }*/
    stop = clock();
    dict_clear(dict);
    double put_call_elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    /*******************************************************************************************************************
     * MEASURE PUTS CALL TIME
     ******************************************************************************************************************/
    size_t *keys = malloc (sizeof(size_t) * NUM_ELEMENTS);
    test_data_t *values = malloc (sizeof(test_data_t) * NUM_ELEMENTS);

    hash_reset_counters(dict);
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        keys[i] = rand();
        values[i] = (test_data_t) {
                .a = 23,
                .b = 42,
                .c = 114
        };
    }

    start = clock();
    dict_puts(dict, NUM_ELEMENTS, keys, values);
    stop = clock();
    free (values);

    double puts_call_elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    /*******************************************************************************************************************
     * MEASURE GET CALL TIME
     ******************************************************************************************************************/

    double get_call_elapsed_keyfound = 0;
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        start = clock();
        dict_get(dict, &keys[i]);
        stop = clock();
        get_call_elapsed_keyfound += (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    }

    double multi_get_call_elapsed_keyfound = get_call_elapsed_keyfound;
    get_call_elapsed_keyfound = get_call_elapsed_keyfound / (double)NUM_ELEMENTS;

    /*******************************************************************************************************************
     * MEASURE GET CALL TIME (NO KEY FOUND)
     ******************************************************************************************************************/

    double get_call_elapsed_nokey = 0;
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        size_t key = rand();
        start = clock();
        dict_get(dict, &key);
        stop = clock();
        get_call_elapsed_nokey += (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    }

    double multi_get_call_elapsed_nokey = get_call_elapsed_nokey;
    get_call_elapsed_nokey = get_call_elapsed_nokey / (double) NUM_ELEMENTS;


    /*******************************************************************************************************************
     * MEASURE GETS CALL TIME
     ******************************************************************************************************************/

    start = clock();
    vec_t *gets_result1 = dict_gets(dict, NUM_ELEMENTS, keys);
    stop = clock();
    vec_free(gets_result1);
    double gets_call_elapsed_keyfound = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;


    /*******************************************************************************************************************
     * MEASURE GETS CALL TIME (NO KEY FOUND)
     ******************************************************************************************************************/

    size_t *other_keys = malloc (sizeof(size_t) * NUM_ELEMENTS);
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        other_keys[i] = rand();
    }

    start = clock();
    vec_t *gets_result2 = dict_gets(dict, NUM_ELEMENTS, other_keys);
    stop = clock();
    vec_free(gets_result2);
    double gets_call_elapsed_nokey = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;





    /*******************************************************************************************************************
     * CLEANUP
     ******************************************************************************************************************/
    free (keys);
    free (other_keys);

    /*******************************************************************************************************************
     * SAVE MEASUREMENTS
     ******************************************************************************************************************/

    linear_hash_table_info_t info;
    hash_table_info(dict, &info);

    printf("%s;%zu;%zu;%0.2f;%0.2f;%0.2f;%zu;%zu;%zu;%zu;%0.4f;%0.4f;%zu;%zu;%zu;%zu;%zu;%zu;%zu;%zu;%zu;%0.8f;%0.8f;%0.8f;%0.8f;%0.8f;%0.8f\n",
           hash_function_name,
           info.num_slots_inuse, info.num_slots_free, info.load_factor,
           info.overhead_size / 1024.0 / 1024.0, info.user_data_size / 1024.0 / 1024.0,
           info.counters.num_locks, info.counters.num_collisions, info.counters.num_rebuilds, info.counters.num_put_calls,
           put_call_elapsed, puts_call_elapsed, info.counters.num_put_slotsearch, info.counters.num_updates,

           info.counters.num_get_foundkey, info.counters.num_get_slotdisplaced, info.counters.num_get_nosuchkey_fullsearch,
           info.counters.num_get_nosuchkey, info.counters.num_test_slot, info.counters.num_slot_get_key,
           info.counters.num_slot_get_value, get_call_elapsed_keyfound, get_call_elapsed_nokey,
           multi_get_call_elapsed_keyfound, multi_get_call_elapsed_nokey,

           gets_call_elapsed_keyfound, gets_call_elapsed_nokey);

    hash_table_free(dict);
}

int main(void)
{
    srand(time(NULL));

    printf("hash_fn;slots_inuse;slots_free;load_factor;footprint_overhead_mib;footprint_user_mib;num_locks;num_collisions;"
                   "num_rebuilds;num_putcalls;elapsed_put_calls_ms;elapsed_puts_calls_ms;num_put_slotsearch;"
                   "num_updates;num_get_foundkey;num_get_slotdisplaced;num_get_nosuchkey_fullsearch;"
                   "num_get_nosuchkey;num_test_slot;num_slot_get_key;num_slot_get_value;get_call_elapsed_keyfound;"
                   "get_call_elapsed_nokey;multi_get_call_elapsed_keyfound;multi_get_call_elapsed_nokey;"
                   "gets_call_elapsed_keyfound;gets_call_elapsed_nokey\n");

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
