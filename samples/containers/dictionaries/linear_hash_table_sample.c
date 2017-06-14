
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
    dictionary_t *dict = linear_hash_table_create(hash_code_size_t, hash_fn_mod, sizeof(size_t), sizeof(test_data_t),
                                                  NUM_SLOTS, GROW_FACTOR);

    for (int i = 0; i < 100000; i++) {
        size_t key = rand();
        test_data_t value = {
            .a = 23,
            .b = 42,
            .c = 114
        };
        dictionary_put(dict, &key, &value);
    }

   // size_t counter = 0;
   // dictionary_for_each(dict, &counter, print_to_stdout);

    linear_hash_table_info_t info;
    linear_hash_table_info(dict, &info);

    printf("slots_inuse;slots_free;load_factor;footprint_overhead_mib;footprint_user_mib;num_locks;num_collisions;num_rebuilds;num_putcalls\n");

    printf("%zu;%zu;%0.2f;%0.2f;%0.2f;%zu;%zu;%zu;%zu\n",
            info.num_slots_inuse, info.num_slots_free, info.load_factor,
            info.overhead_size / 1024.0 / 1024.0, info.user_data_size / 1024.0 / 1024.0,
            info.counters.num_locks, info.counters.num_collisions, info.counters.num_rebuilds, info.counters.num_put_calls);

    linear_hash_table_free(dict);


    return 0;

}
