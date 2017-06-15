
#include <stddef.h>
#include <stdlib.h>
#include <defs.h>
#include <time.h>
#include <containers/vector.h>
#include <containers/dictionaries/fixed_linear_hash_table.h>

const size_t SIZEOF_KEY = 600;
const size_t NUM_SLOTS = 1000;
const float  MAX_LOAD_FACTOR = 0.5f;
const size_t SIZEOF_VALUE = sizeof(size_t);

#define clear_buffer(buffer, size)                                                                                     \
    memset(buffer, 0, size);                                                                                           \

#define set_buffer_from_list(buffer, src, word_id, size)                                                               \
    clear_buffer(buffer, size)                                                                                         \
    memcpy(buffer, src + word_id * size, size);

#define set_buffer_direct(buffer, src, size)                                                                           \
    clear_buffer(buffer, size)                                                                                         \
    memcpy(buffer, src, size);

static vector_t *read_all_lines(const char *file)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    vector_t *result = vector_create(SIZEOF_KEY, 69199328);

    fp = fopen(file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    size_t max_len = 0;
    char buffer[SIZEOF_KEY];
    while ((read = getline(&line, &len, fp)) != -1) {
        max_len = max(max_len, len);
        if (len > 2) {
            line[strlen(line) - 1] = '\0'; // removing the newline character
            set_buffer_direct(buffer, line, min(strlen(line) + 1, SIZEOF_KEY));

            vector_add(result, 1, &buffer);
        }
    }

    fclose(fp);
    if (line)
        free(line);

    printf("maximum length of word: %zu\n", max_len);

    return result;
}



int main(void)
{
    vector_t *words = read_all_lines("/Users/marcus/temp/linux-words-tiny");
    dict_t *dict = fixed_linear_hash_table_create(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                                  SIZEOF_KEY, SIZEOF_VALUE, NUM_SLOTS, GROW_FACTOR, MAX_LOAD_FACTOR);

    char key[SIZEOF_KEY];

    for (int i = 0; i < words->num_elements; i++) {
        set_buffer_from_list(key, words->data, i, SIZEOF_KEY);

        const void *value = dict_get(dict, key);
        size_t count = (value == NULL) ? 1 : ((*(size_t *) value) + 1);
        dict_put(dict, key, &count);
    }

    char *query = "following", *result;
    set_buffer_direct(key, query, strlen(query) + 1);

    if ((result = (char *) dict_get(dict, key)) != NULL) {
        printf("#%s = %zu\n", result, *(size_t *)result );
    };


    return 0;

}
