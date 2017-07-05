
#include <stddef.h>
#include <stdlib.h>
#include <printf.h>

int main(int argc, char* argv[]) {

    if (argc != 5) {
        printf("First parameter must be nice-level and second must be start-num of tuples, third must be end-num of tuples, 4th must be step size of tuples\n\t example: 19 22002906 11001453114 548972510 (nice level 19, start with approx. 10GiB DB size, end with 5 TiB DB size, use approx 20 steps");
        abort();
    }

    char *nice_level = argv[1];
    int start_tuples = atoi(argv[2]);
    int max_tuples = atoi(argv[3]);
    int step_tuples = atoi(argv[4]);

    printf("nice_level;full;num_tuples;db_size_mb;it;result_set_size_mb;exec_time\n");

    for (size_t num_tuples = start_tuples; num_tuples < max_tuples; num_tuples += step_tuples) {

        size_t MAX_ITERATIONS = 200;
        const size_t NUM_COLUMNS = 61; // num of TPC-H columns

        size_t **all_columns = malloc(NUM_COLUMNS * sizeof(size_t *));
        for (int i = 0; i < NUM_COLUMNS; i++) {
            all_columns[i] = malloc(num_tuples * sizeof(size_t));
            for (int j = 0; j < num_tuples; j++) {
                all_columns[i][j] = rand();
            }
        }

        for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
            size_t *result_set = malloc(num_tuples * sizeof(size_t));
            size_t pos = 0;
            clock_t start = clock();
            for (int tid = 0; tid < num_tuples; tid++) {
                if (all_columns[0][tid] % 2 == 0) {
                    result_set[pos++] = tid;
                }
            }
            clock_t stop = clock();
            double elapsed = (double) (stop - start) * 1000.0 / CLOCKS_PER_SEC;
            free(result_set);
            printf("%s;yes;%zu;%0.4f;%d;%0.4f;%0.4f\n", nice_level, num_tuples,
                   (num_tuples * NUM_COLUMNS * sizeof(size_t) / 1024 / 1024.0), iteration,
                   (pos * sizeof(size_t)) / 1024 / 1024.0, elapsed);
        }

        for (int i = 0; i < NUM_COLUMNS; i++) {
            free(all_columns[i]);
        }
        free(all_columns);


        // Only one column in order to minimize swapping

        all_columns = malloc(sizeof(size_t *));
        all_columns[0] = malloc(num_tuples * sizeof(size_t));
        for (int j = 0; j < num_tuples; j++) {
            all_columns[0][j] = rand();
        }

        for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
            size_t *result_set = malloc(num_tuples * sizeof(size_t));
            size_t pos = 0;
            clock_t start = clock();
            for (int tid = 0; tid < num_tuples; tid++) {
                if (all_columns[0][tid] % 2 == 0) {
                    result_set[pos++] = tid;
                }
            }
            clock_t stop = clock();
            double elapsed = (double) (stop - start) * 1000.0 / CLOCKS_PER_SEC;
            free(result_set);
            printf("%s;no;%zu;%0.4f;%d;%0.4f;%0.4f\n", nice_level, num_tuples,
                   (num_tuples * NUM_COLUMNS * sizeof(size_t) / 1024 / 1024.0), iteration,
                   (pos * sizeof(size_t)) / 1024 / 1024.0, elapsed);
        }

        free(all_columns[0]);
        free(all_columns);
    }


    return 0;
}