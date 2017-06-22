
#include <stddef.h>
#include <stdlib.h>
#include <defs.h>
#include <time.h>
#include <containers/vector.h>
#include <containers/dictionaries/fixed_linear_hash_table.h>

const size_t SIZEOF_KEY = 100;
const size_t NUM_SLOTS = 10000;
const float  MAX_LOAD_FACTOR = 0.75f;
const size_t SIZEOF_VALUE = sizeof(size_t);

#define clear_buffer(buffer, size)                                                                                     \
    memset(buffer, 0, size);                                                                                           \

#define set_buffer_from_list(buffer, src, word_id, size)                                                               \
    clear_buffer(buffer, size)                                                                                         \
    strcpy(buffer, src + word_id * size);

#define set_buffer_direct(buffer, src, size)                                                                           \
    clear_buffer(buffer, size)                                                                                         \
    strcpy(buffer, src);

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
            line[min(strlen(line) - 1, SIZEOF_KEY - 1)] = '\0'; // removing the newline character
            set_buffer_direct(buffer, line, SIZEOF_KEY);

            vector_add(result, 1, &buffer);
        }
    }

    fclose(fp);
    if (line)
        free(line);

    printf("maximum length of word: %zu\n", max_len);

    return result;
}

void query(char *key, dict_t *dict, const char *query)
{
    const char *result;
    set_buffer_direct(key, query, SIZEOF_KEY);

    clock_t start = clock();
    result = (char *) dict_get(dict, key);
    clock_t stop = clock();
    double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    printf("query '%s' \n>> ", query);
    if (result!= NULL) {
        printf("%zu occurrences (execution time: %0.04fms)", *(size_t *)result, elapsed);
    };
    printf("\n");
}

int main(void)
{
    dict_t *dict = fixed_linear_hash_table_create(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                                  SIZEOF_KEY, SIZEOF_VALUE, NUM_SLOTS, 1.7f, MAX_LOAD_FACTOR);

    // TODO: The file can be downloaded here: https://www.dropbox.com/sh/kf5sbw74rru3kco/AAB07Cwy0oVbRih33nef_FTFa?dl=0
    vector_t *words = read_all_lines("/Users/marcus/temp/linux-words");


    char *key = malloc(SIZEOF_KEY);

    clock_t start = clock();
    for (int i = 0; i < words->num_elements; i++) {
        set_buffer_from_list(key, words->data, i, SIZEOF_KEY);
        const void *value = dict_get(dict, key);
        size_t count = (value == NULL) ? 1 : ((*(size_t *) value) + 1);
        dict_put(dict, key, &count);
        linear_hash_table_info_t info;
        fixed_linear_hash_table_info(dict, &info);
      //  printf("%0.4f%% load factor: %0.4f%%, num rebuilds %zu, num slots:%zu\n", (i / (float) words->num_elements) * 100, info.load_factor, info.counters.num_rebuilds, info.num_slots_inuse + info.num_slots_free);
    }
    clock_t stop = clock();
    double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("It tooks %0.4fms to load %zu words\n", elapsed, words->num_elements);


    query(key, dict, "auto");
    query(key, dict, "break");
    query(key, dict, "case");
    query(key, dict, "char");
    query(key, dict, "const");
    query(key, dict, "continue");
    query(key, dict, "default");
    query(key, dict, "do");
    query(key, dict, "double");
    query(key, dict, "else");
    query(key, dict, "enum");
    query(key, dict, "extern");
    query(key, dict, "float");
    query(key, dict, "for");
    query(key, dict, "goto");
    query(key, dict, "if");
    query(key, dict, "int");
    query(key, dict, "long");
    query(key, dict, "register");
    query(key, dict, "return");
    query(key, dict, "short");
    query(key, dict, "signed");
    query(key, dict, "sizeof");
    query(key, dict, "static");
    query(key, dict, "struct");
    query(key, dict, "switch");
    query(key, dict, "typedef");
    query(key, dict, "union");
    query(key, dict, "unsigned");
    query(key, dict, "void");
    query(key, dict, "volatile");
    query(key, dict, "while");

    query(key, dict, "Linux");
    query(key, dict, "Linus");
    query(key, dict, "Torvalds");



    return 0;

}
