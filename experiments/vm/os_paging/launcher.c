
#include <stddef.h>
#include <stdlib.h>
#include <printf.h>
#include <sys/statvfs.h>

#include <unistd.h>
#include <stdinc.h>
#include <containers/dict.h>
#include <containers/dicts/hash_table.h>
#include <containers/vec.h>

char * exec_cmd(const char *cmd) {

    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    char *out = malloc (2048);
    size_t off = 0;

    stream = popen(cmd, "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) {
                strcpy(out + off, buffer);
                off += strlen(buffer);
            }
        pclose(stream);
    }
    return out;
}

struct sysctrl_info {
    char *total, *used, *free_mem;
};

struct sysctrl_info sysctrl_data() {

    char *out = exec_cmd("/usr/sbin/sysctl vm.swapusage 2>&1");

    size_t total_start = 0;
    size_t total_len = 0;

    size_t free_start = 0;
    size_t free_len = 0;

    size_t used_start = 0;
    size_t used_len = 0;

    int mode = 0;

    for (char *it = out; *it; it++) {
        if (mode == 0) {
            if (*it == '=') {
                it++;
                it++;
                total_start = (it - out);
                mode = 1;
            }
        } else if (mode == 1) {
            if (*it == 'M') {
                it++;
                total_len = (it - out) - total_start;
                mode = 2;
            }
        } else if (mode == 2) {
            if (*it == '=') {
                it++;
                it++;
                used_start = (it - out);
                mode = 3;
            }
        } else if (mode == 3) {
            if (*it == 'M') {
                it++;
                used_len = (it - out) - used_start;
                mode = 4;
            }
        } else if (mode == 4) {
            if (*it == '=') {
                it++;
                it++;
                free_start = (it - out);
                mode = 5;
            }
        } else if (mode == 5) {
            if (*it == 'M') {
                it++;
                free_len = (it - out) - free_start;
                mode = 6;
            }
        }
    }

    char *total = malloc (total_len);
    memcpy(total, out + total_start, total_len);
    total[total_len - 1] = '\0';

    char *used = malloc (used_len);
    memcpy(used, out + used_start, used_len);
    used[used_len - 1] = '\0';

    char *free_mem = malloc (free_len);
    memcpy(free_mem, out + free_start, free_len);
    free_mem[free_len - 1] = '\0';


    free (out);

    return (struct sysctrl_info) {
        .total = total,
        .free_mem = free_mem,
        .used = used
    };
}

typedef struct {
    size_t lhs, rhs;
} pair_t;

typedef struct {
    size_t *filter_result_set;
    dict_t *hash_table;
    pair_t *final_result_set;


    size_t filter_result_set_elem_num;
    size_t max_filter_result_set_elem_num;

    size_t max_hash_table_elem_num;
    size_t max_final_result_set_elem_num;

    size_t filter_result_set_size;
    size_t final_result_set_size;
} out_t;

void process_query(out_t *out, size_t num_tuples, size_t *lhs_column, size_t *rhs_column)
{
    size_t max_filter_result_set_size = 0.11f * num_tuples + 1;
    // filter
    size_t *filter_result_set = malloc(max_filter_result_set_size * sizeof(size_t));
    size_t filter_result_set_size = 0;
    for (int tid = 0; tid < num_tuples; tid++) {
        if (lhs_column[tid] < 1) {
            filter_result_set[filter_result_set_size++] = tid;
            assert (filter_result_set_size < max_filter_result_set_size);
        }
    }

    size_t max_hash_table_size = max_filter_result_set_size * 1.1f;
    dict_t *hash_table = hash_table_create(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                          sizeof(size_t), sizeof(size_t), max_hash_table_size, 1.7f, 0.99f);
    // hash phase
    for (int i = 0; i < filter_result_set_size; i++) {
        size_t tid = filter_result_set[i];
        size_t val = lhs_column[tid];
        dict_put(hash_table, &val, &tid);
        assert (dict_num_elements(hash_table) < max_hash_table_size);
    }

    size_t max_final_result_set_size = filter_result_set_size * 1.1f;
    pair_t *final_result_set = malloc(max_final_result_set_size * sizeof(pair_t));
    size_t final_result_set_size = 0;

    // probe phase
    for (int rhs_tid = 0; rhs_tid < num_tuples; rhs_tid++) {
        size_t val = lhs_column[rhs_tid];
        const void *lhs_tid = NULL;
        if ((lhs_tid = dict_get(hash_table, &val)) != NULL) {
            final_result_set[final_result_set_size++] = (pair_t) {.lhs = *(size_t *) lhs_tid, .rhs = rhs_tid};
            assert (final_result_set_size < max_final_result_set_size);
        }
    }

    out->filter_result_set = filter_result_set;
    out->hash_table = hash_table;
    out->final_result_set = final_result_set;
    out->filter_result_set_elem_num = filter_result_set_size;
    out->max_filter_result_set_elem_num = max_filter_result_set_size;
    out->max_hash_table_elem_num = max_hash_table_size;
    out->max_final_result_set_elem_num = max_final_result_set_size;
    out->filter_result_set_size = filter_result_set_size;
    out->final_result_set_size = final_result_set_size;
}

typedef size_t s_nationkey_t;
typedef size_t n_nationkey_t;
typedef size_t ps_suppkey_t;
typedef size_t s_suppkey_t;
typedef size_t ps_supplycost_t;
typedef size_t ps_availqty_t;
typedef size_t ps_partkey_t;
typedef double number_t;
typedef size_t tid_t;
typedef char * n_name_t;

typedef struct {
    tid_t lhs, rhs;
} tid_pair_t;

vector_t *global_intermediate_column_sum = NULL;

int comp_tid_by_value(const void *tid_lhs, const void *tid_rhs) {
    number_t value_left = *(number_t*) vector_at(global_intermediate_column_sum, *(tid_t *) tid_lhs);
    number_t value_right = *(number_t*) vector_at(global_intermediate_column_sum, *(tid_t *) tid_rhs);
    return (value_left < value_right ? - 1 : (value_left > value_right ? + 1 : 0));
}

typedef struct {
    tid_t            nations_num_tuples;
    n_nationkey_t   *n_nationkey;
    n_name_t        *n_name;
} tab_nation_t;

typedef struct {
    tid_t            supplier_num_tuples;
    s_nationkey_t   *s_nationkey;
    s_suppkey_t     *s_suppkey;
} tab_supplier_t;

typedef struct {
    tid_t            partsupp_num_tuples;
    ps_suppkey_t    *ps_suppkey;
    ps_supplycost_t *ps_supplycost;
    ps_availqty_t   *ps_availqty;
    ps_partkey_t    *ps_partkey;
} tab_partsupp_t;


void process_query_11(
    tab_nation_t    *tab_nation,
    tab_supplier_t  *tab_supplier,
    tab_partsupp_t  *tab_partsupp)
{

    tid_t            nations_num_tuples   = tab_nation->nations_num_tuples;
    n_nationkey_t   *n_nationkey          = tab_nation->n_nationkey;
    n_name_t        *n_name               = tab_nation->n_name;

    tid_t            supplier_num_tuples  = tab_supplier->supplier_num_tuples;
    s_nationkey_t   *s_nationkey          = tab_supplier->s_nationkey;
    s_suppkey_t     *s_suppkey            = tab_supplier->s_suppkey;

    tid_t            partsupp_num_tuples  = tab_partsupp->partsupp_num_tuples;
    ps_suppkey_t    *ps_suppkey           = tab_partsupp->ps_suppkey;
    ps_supplycost_t *ps_supplycost        = tab_partsupp->ps_supplycost;
    ps_availqty_t   *ps_availqty          = tab_partsupp->ps_availqty;
    ps_partkey_t    *ps_partkey           = tab_partsupp->ps_partkey;

    // TPC-H Query 11
    //
    //     Choosen: Non trivial query involving same tables in different stages of the plan
    //
    //     Physical plan:
    //
    //     materialize(
    //         sort(
    //              nested_loop_join(
    //                  map(
    //                      group_by(
    //                          hash_join(
    //                              hash_join(
    //                                  filter(
    //                                      NATION,
    //                                      (n_name == "GERMANY")
    //                                  ),
    //                                  SUPPLIER
    //                              ),
    //                              PARTSUPP
    //                          )
    //                      )
    //                  ),
    //                  group_by(
    //                      hash_join(
    //                          hash_join(
    //                              NATION,
    //                              SUPPLIER
    //                              ),
    //                              PARTSUPP
    //                          )
    //                  )
    //              )
    //      )
    //
    //

    /* filter NATION by (n_name == "GERMANY") */
    vector_t *filter_nation_n_name_germany_tids = vector_create(sizeof(tid_t), 1);
    for (tid_t tid = 0; tid < nations_num_tuples; tid++) {
        if (strcmp(n_name[tid], "GERMANY"))
            vector_add(filter_nation_n_name_germany_tids, 1, &tid);
    }


    /* Calcuate hash_join(filtered(NATION), SUPPLIER, (s_nationkey == n_nationkey) */
    size_t approx_join_nation_supplier_size = supplier_num_tuples * 0.06f;

    size_t filtered_nations_tuple_num = filter_nation_n_name_germany_tids->num_elements;

  //  dict_t *hash_nation_supplier = hash_table_create(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
  //                                                   sizeof(s_nationkey_t), sizeof(tid_t), filtered_nations_tuple_num * 10, 1.7f, 0.75f);

    vector_t *join_nation_supplier_tid_pairs = vector_create_ex(sizeof(tid_t), approx_join_nation_supplier_size,
                                                                auto_resize, 1.7f);

    // table scan
    for (tid_t supplier_tid = 0; supplier_tid < supplier_num_tuples; supplier_tid++) {
        for (size_t i = 0; i < filtered_nations_tuple_num; i++) {
            tid_t nation_tid = *(tid_t *) vector_at(filter_nation_n_name_germany_tids, i);
            if (n_nationkey[nation_tid] == s_nationkey[supplier_tid]) {
                vector_add(join_nation_supplier_tid_pairs, 1, &supplier_tid);
            }
        }
    }

    vector_free (filter_nation_n_name_germany_tids);

    size_t approx_join_nation_supplier_partsupp_tids_length = partsupp_num_tuples * 0.06f;
    vector_t *join_nation_supplier_partsupp_tid_pairs = vector_create_ex(sizeof(tid_t),
                                                                         approx_join_nation_supplier_partsupp_tids_length,
                                                                         auto_resize, 1.7f);

    size_t joined_nation_supplier_tid_pairs_length = join_nation_supplier_tid_pairs->num_elements;
    for (size_t i = 0; i < joined_nation_supplier_tid_pairs_length; i++) {
        tid_t supplier_tid = *(tid_t *) vector_at(join_nation_supplier_tid_pairs, i);
        for (tid_t partsupp_tid = 0; partsupp_tid < partsupp_num_tuples; partsupp_tid++) {
            if (s_suppkey[supplier_tid] == ps_suppkey[partsupp_tid]) {
                vector_add(join_nation_supplier_partsupp_tid_pairs, 1, &partsupp_tid);
            }
        }
    }

    vector_free (join_nation_supplier_tid_pairs);

    // sum
    number_t sum = 0;
    for (size_t i = 0; i < join_nation_supplier_partsupp_tid_pairs->num_elements; i++) {
        tid_t joined_filtered_partsupp_tid = *(tid_t *) vector_at(join_nation_supplier_partsupp_tid_pairs, i);
        sum += (ps_supplycost[joined_filtered_partsupp_tid] * ps_availqty[joined_filtered_partsupp_tid]);
    }
    sum *= 0.0001f;

    size_t approx_hash_table_size = 100;

    // grouping with hashing
    dict_t *hash_table = hash_table_create(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                           sizeof(ps_partkey_t), sizeof(vector_t *), approx_hash_table_size, 1.7f, 0.75f);

    for (size_t i = 0; i < join_nation_supplier_partsupp_tid_pairs->num_elements; i++) {
        tid_t partsupp_tid = *(tid_t *) vector_at(join_nation_supplier_partsupp_tid_pairs, i);
        if ((ps_supplycost[partsupp_tid] * ps_availqty[partsupp_tid]) > sum) {
            ps_partkey_t partkey = ps_partkey[partsupp_tid];

            if (!dict_contains_key(hash_table, &partkey)) {
                size_t approx_matching_tid_num = 10;

                vector_t *matching_tids = vector_create_ex(sizeof(tid_t),
                                                           approx_matching_tid_num,
                                                           auto_resize, 1.7f);

                dict_put(hash_table, &partkey, &matching_tids);
            }

            vector_t *matching_tids = (vector_t *) dict_get(hash_table, &partkey);
            assert (matching_tids != NULL);
            vector_add(matching_tids, 1, &partsupp_tid);
        }
    }

    vector_free (join_nation_supplier_partsupp_tid_pairs);

    // final selection

    const vector_t *partkey_tids = dict_keyset(hash_table);

    vector_t *intermediate_tuple_id_list = vector_create_ex(sizeof(tid_t),
                                                            partkey_tids->num_elements,
                                                            auto_resize, 1.7f);

    vector_t *intermediate_ps_partkey = vector_create_ex(sizeof(ps_partkey_t),
                                                         partkey_tids->num_elements,
                                                         auto_resize, 1.7f);

    vector_t *intermediate_column_sum = vector_create_ex(sizeof(number_t),
                                                         partkey_tids->num_elements,
                                                         auto_resize, 1.7f);



    tid_t intermedtia_tuple_id = 0;

    for (size_t i = 0; i < partkey_tids->num_elements; i++) {
        ps_partkey_t partkey = *(ps_partkey_t *) vector_at(partkey_tids, i);
        vector_t *matching_tids = (vector_t *) dict_get(hash_table, &partkey);
        for (size_t j = 0; j < partkey_tids->num_elements; j++) {
            tid_t partsupp_tid = *(tid_t *) vector_at(matching_tids, j);
            number_t intermedita_sum = ps_supplycost[partsupp_tid] * ps_availqty[partsupp_tid];

            vector_add(intermediate_tuple_id_list, 1, &intermedtia_tuple_id);
            vector_add(intermediate_ps_partkey, 1, &partkey);
            vector_add(intermediate_column_sum, 1, &intermedita_sum);

            intermedtia_tuple_id++;
        }
    }

    dict_free(hash_table);

    // no sort
    global_intermediate_column_sum = intermediate_column_sum;

    qsort(intermediate_tuple_id_list->data, intermediate_tuple_id_list->num_elements, intermediate_tuple_id_list->sizeof_element,
          comp_tid_by_value);


    // Materialize for DEBUG
    printf("| ps_partkey\t | value |\n");
    printf("+--------------+-------|\n");
    for (size_t i = 0; i < intermediate_tuple_id_list->num_elements; i++) {
        tid_t tid = *(tid_t *) vector_at(intermediate_tuple_id_list, i);
        ps_partkey_t partkey = *(ps_partkey_t *) vector_at(intermediate_ps_partkey, tid);
        number_t intermedita_sum = *(number_t *) vector_at(intermediate_column_sum, tid);

        printf("| %zu\t | %0.04f |\n", partkey, intermedita_sum);
    }
    printf("+--------------+-------|\n\t%zu records\n", intermediate_tuple_id_list->num_elements);

    vector_free(intermediate_tuple_id_list);
    vector_free(intermediate_ps_partkey);
    vector_free(intermediate_column_sum);
}

void benchmark_tph_query_11()
{
    tab_nation_t     tab_nation   = {

    };

    tab_supplier_t   tab_supplier = {

    };

    tab_partsupp_t   tab_partsupp = {

    };
}

int main(int argc, char* argv[]) {

    size_t MAX_ITERATIONS = 25;

    if (argc != 5) {
        printf("First parameter must be nice-level and second must be start-num of tuples, third must be end-num of tuples, 4th must be step size of tuples\n\t example: 19 22002906 11001453114 548972510 (nice level 19, start with approx. 10GiB DB size, end with 5 TiB DB size, use approx 20 steps\n\n");

        for (int i = 4000000; i < 400000000; i+=4000000) {
            printf("./exp_os_paging 0 %d %d %d >> results_complex_multi.csv;\n", i, i + 1, i + 100);
        }

        printf("nice_level;full;num_tuples;db_size_mb;it;exec_time;disk_freespace;swap_total_mb;swap_used_mb;swap_free_mb;filter_result_set_elem_num;max_filter_result_set_elem_num;max_hash_table_elem_num;max_final_result_set_elem_num;filter_result_set_size;hash_table_num_elem;final_result_set_size\n");

        exit(0);
    }

    char *nice_level = argv[1];
    int start_tuples = atoi(argv[2]);
    int max_tuples = atoi(argv[3]);
    int step_tuples = atoi(argv[4]);



    for (size_t num_tuples = start_tuples; num_tuples < max_tuples; num_tuples += step_tuples) {

        const size_t NUM_COLUMNS = 16; // num of TPC-H columns for 'LINEITEM'

        size_t **all_columns = NULL;

        // Only one column in order to minimize swapping

        all_columns = malloc(sizeof(size_t *));
        all_columns[0] = malloc(num_tuples * sizeof(size_t));
        size_t val = 0;
        for (int j = 0; j < num_tuples; j++) {
            all_columns[0][j] = (val++) % 10;
        }

        for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {

            clock_t start = clock();

            out_t out;
            process_query(&out, num_tuples, all_columns[0], all_columns[0]);

            process_query_11()

            clock_t stop = clock();
            double elapsed = (double) (stop - start) * 1000.0 / CLOCKS_PER_SEC;

            struct statvfs st;
            statvfs("/", &st);
            unsigned long free_space = st.f_bfree * st.f_frsize;
            struct sysctrl_info info = sysctrl_data();

            size_t hash_num_elem = dict_num_elements(out.hash_table);

            printf("%s;no;%zu;%0.4f;%d;%0.4f;%zu;%s;%s;%s;%zu;%zu;%zu;%zu;%zu;%zu;%zu\n", nice_level, num_tuples,
                   (num_tuples * NUM_COLUMNS * sizeof(size_t) / 1024 / 1024.0), iteration, elapsed, free_space,
                   info.total, info.used, info.free_mem,
                   out.filter_result_set_elem_num,
                   out.max_filter_result_set_elem_num,
                   out.max_hash_table_elem_num,
                   out.max_final_result_set_elem_num,
                   out.filter_result_set_size,
                   hash_num_elem,
                   out.final_result_set_size);

            free(out.filter_result_set);
            dict_free(out.hash_table);
            free(out.final_result_set);
            free (info.total);
            free (info.free_mem);
            free (info.used);
        }

        free(all_columns[0]);
        free(all_columns);

        // all columns


        all_columns = malloc(NUM_COLUMNS * sizeof(size_t *));
        for (int i = 0; i < NUM_COLUMNS; i++) {
            all_columns[i] = malloc(num_tuples * sizeof(size_t));
            for (int j = 0; j < num_tuples; j++) {
                all_columns[0][j] = (val++) % 10;
            }
        }

        for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
            clock_t start = clock();

            out_t out;
            process_query(&out, num_tuples, all_columns[0], all_columns[NUM_COLUMNS - 1]);

            clock_t stop = clock();
            double elapsed = (double) (stop - start) * 1000.0 / CLOCKS_PER_SEC;

            struct statvfs st;
            statvfs("/", &st);
            unsigned long free_space = st.f_bfree * st.f_frsize;
            struct sysctrl_info info = sysctrl_data();


            printf("%s;yes;%zu;%0.4f;%d;%0.4f;%zu;%s;%s;%s;%zu;%zu;%zu;%zu;%zu;%zu;%zu\n", nice_level, num_tuples,
                   (num_tuples * NUM_COLUMNS * sizeof(size_t) / 1024 / 1024.0), iteration, elapsed, free_space,
                   info.total, info.used, info.free_mem,
                   out.filter_result_set_elem_num,
                   out.max_filter_result_set_elem_num,
                   out.max_hash_table_elem_num,
                   out.max_final_result_set_elem_num,
                   out.filter_result_set_size,
                   dict_num_elements(out.hash_table),
                   out.final_result_set_size);

            free(out.filter_result_set);
            dict_free(out.hash_table);
            free(out.final_result_set);
            free (info.total);
            free (info.free_mem);
            free (info.used);
        }

        for (int i = 0; i < NUM_COLUMNS; i++) {
            free(all_columns[i]);
        }
        free(all_columns);



    }


    return 0;
}