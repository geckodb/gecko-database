
#include <storage/database.h>
#include <storage/timespan.h>

#define NUM_EXISTING_STRINGS  10
#define NUM_NEW_STRINGS       10000

// Taken from: https://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
long random_at_most(long max) {
    unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
            num_bins = (unsigned long) max + 1,
            num_rand = (unsigned long) RAND_MAX + 1,
            bin_size = num_rand / num_bins,
            defect   = num_rand % num_bins;

    long x;
    do {
        x = random();
    }
        // This is carefully written not to overflow
    while (num_rand - defect <= (unsigned long)x);

    // Truncated division is intentional
    return x/bin_size;
}

int main() {
    int num_new_nodes = 1;

    gs_timer_t timer_nodes_created, timer_strings_existing_created_1, timer_strings_existing_created_2, timer_strings_existing_created_3,
            timer_strings_existing_created_4, timer_strings_existing_created_5, timer_strings_existing_created_6,
            timer_strings_existing_created_7, timer_strings_existing_created_8, timer_strings_existing_created_9,
            timer_strings_existing_created_10, timer_strings_find_existing_new,
            timer_strings_non_existing_created, timer_strings_read, timer_strings_fullscan, timer_strings_fullscan_read, timer_strings_find_non_existing,
            timer_strings_find_existing_1, timer_strings_find_existing_2, timer_strings_find_existing_3;

    database_t *database;
    database_open(&database, "/Users/marcus/temp/db");
    gs_status_t status;
    timespan_t lifetime = { .begin = 0, .end = 200};

    gs_timer_start(&timer_nodes_created);
    node_id_t *node_records = database_nodes_create(&status, database, num_new_nodes, &lifetime);
    gs_timer_stop(&timer_nodes_created);

    database_node_cursor_t *cursor;
    database_nodes_fullscan(&cursor, database);
    database_node_cursor_open(cursor);

    while (database_node_cursor_next(cursor)) {
        database_node_t node;
        database_node_cursor_read(&node, cursor);
        printf("node id: %lld [%lld, %lld) LATEST: %s CREATED: %lld\t",
               node.unique_id, node.lifetime.begin, node.lifetime.end,
               node.flags.has_version_next ? "N" : "Y", node.creation_time);

        database_node_version_cursor_t *version_cursor;
        database_node_version_cursor_open(&version_cursor, &node, database);
        while (database_node_version_cursor_next(version_cursor)) {
            database_node_t node_version;
            database_node_version_cursor_read(&node_version, version_cursor);

            printf("node id: %lld [%lld, %lld) (LATEST: %s) CREATED: %lld\t",
                   node_version.unique_id, node_version.lifetime.begin, node_version.lifetime.end,
                   node_version.flags.has_version_next ? "N" : "Y", node_version.creation_time);

        }
        database_node_version_cursor_close(version_cursor);
        printf("\n");
    }

    database_node_cursor_close(cursor);

    for (int i = 0; i < 10; i++) {
        lifetime.begin = 10 + i;
        lifetime.end = 300 + i;
        database_nodes_alter_lifetime(database, node_records, num_new_nodes, &lifetime)  ;
    }


    srandom(0);

    unsigned num_strings = NUM_EXISTING_STRINGS;
    const char **strings = malloc(num_strings * sizeof(char *));
    for (int i = 0; i < num_strings; i++) {
        long size = 1 + random_at_most(100);
        strings[i] = malloc(size + 1);
        for (int j = 0; j  < size; j++) {
            ((char *) strings[i])[j] = 'a' + random_at_most(27);
        }
        printf("new existing string... '%s'\n", strings[i]);
    }

    gs_status_t status_add_str;
    size_t num_created_strings;

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_1);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    string_id_t *strings_added = database_string_create(&num_created_strings, &status_add_str, database, strings,
                                                        num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_1);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_2);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                                                        num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_2);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_3);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                                                        num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_3);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_4);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                           num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_4);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_5);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                           num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_5);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_6);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                           num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_6);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_7);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                           num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_7);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_8);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                           num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_8);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_9);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                           num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_9);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_existing_created_10);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    database_string_create(&num_created_strings, &status_add_str, database, strings,
                           num_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_existing_created_10);

    //------------------------------------------------------------------------------------------------------------------

    srandom(time(NULL));

    unsigned num_new_strings = NUM_NEW_STRINGS;
    const char **new_strings = malloc(num_new_strings * sizeof(char *));
    for (int i = 0; i < num_new_strings; i++) {
        long size = 1 + random_at_most(100);
        new_strings[i] = malloc(size + 1);
        for (int j = 0; j  < size; j++) {
            ((char *) new_strings[i])[j] = 'a' + random_at_most(27);
        }
        printf("new string... '%s'\n", new_strings[i]);
    }

    gs_status_t status_add_str_new;
    size_t num_created_strings_new;

    gs_timer_start(&timer_strings_non_existing_created);
    /* Each string is intended to be stored in the cache due to 'buffer_alloc_fetch' */
    string_id_t *strings_added_new = database_string_create(&num_created_strings_new, &status_add_str_new, database, new_strings,
                                                            num_new_strings, memorization_policy_keep_in_cache);
    gs_timer_stop(&timer_strings_non_existing_created);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_read);
    database_string_t *strings_added_strs = database_string_read(NULL, database, strings_added_new, num_created_strings_new);
    gs_timer_stop(&timer_strings_read);

    for (int i = 0; i < num_created_strings_new; i++) {
             printf("Created in db: '%lld: %s'\n", strings_added_new[i], strings_added_strs[i].string);
    }

    size_t num_strings_in_db;
    gs_timer_start(&timer_strings_fullscan);
    string_id_t *all_string_ids = database_string_fullscan(&num_strings_in_db, NULL, database);
    gs_timer_stop(&timer_strings_fullscan);

    gs_timer_start(&timer_strings_fullscan_read);
    database_string_t *loaded_strings = database_string_read(NULL, database, all_string_ids, num_strings_in_db);
    gs_timer_stop(&timer_strings_fullscan_read);

    for (int i = 0; i < num_strings_in_db; i++) {
        printf("Load from db: '%lld: %s'\n", all_string_ids[i], loaded_strings[i].string);
    }

    //------------------------------------------------------------------------------------------------------------------

    size_t num_strings_found = 0;

    unsigned num_unknown_strings = 10;
    char **unknown_strings = malloc(num_unknown_strings * sizeof(char *));
    for (int i = 0; i < num_unknown_strings; i++) {
        long size = 1 + random_at_most(100);
        unknown_strings[i] = malloc(size + 1);
        for (int j = 0; j  < size; j++) {
            ((char *) unknown_strings[i])[j] = 'a' + random_at_most(27);
        }
        printf("unknown string... '%s'\n", unknown_strings[i]);
    }

    gs_timer_start(&timer_strings_find_non_existing);
    string_id_t *found_str_ids;
    database_string_find(&num_strings_found, &found_str_ids, NULL, database, unknown_strings, num_unknown_strings);
    gs_timer_stop(&timer_strings_find_non_existing);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_find_existing_new);
    database_string_find(&num_strings_found, &found_str_ids, NULL, database, (char **) new_strings, num_new_strings);
    gs_timer_stop(&timer_strings_find_existing_new);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_find_existing_1);
    database_string_find(&num_strings_found, &found_str_ids, NULL, database, (char **) strings, num_strings);
    gs_timer_stop(&timer_strings_find_existing_1);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_find_existing_2);
    database_string_find(&num_strings_found, &found_str_ids, NULL, database, (char **) strings, num_strings);
    gs_timer_stop(&timer_strings_find_existing_2);

    //------------------------------------------------------------------------------------------------------------------

    gs_timer_start(&timer_strings_find_existing_3);
    database_string_find(&num_strings_found, &found_str_ids, NULL, database, (char **) strings, num_strings);
    gs_timer_stop(&timer_strings_find_existing_3);

    //------------------------------------------------------------------------------------------------------------------

    database_string_t *strings_found = database_string_read(NULL, database, found_str_ids, num_strings_found);

   // for (int i = 0; i < num_strings_found; i++) {
   //     printf("Load from db: '%lld: %s'\n", found_str_ids[i], strings_found[i].string);
   // }

    printf("\n-------------------------------------------\n"
           "timer_nodes_created:\t\t\t\t%lldmsec\n"
           "Time create existing strings: \t\t%lldmsec \t\t\t\t\t\t(%0.4fmsec per string)\n"
           "Time create existing strings (pass 2): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
           "Time create existing strings (pass 3): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
                   "Time create existing strings (pass 4): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
                   "Time create existing strings (pass 5): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
                   "Time create existing strings (pass 6): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
                   "Time create existing strings (pass 7): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
                   "Time create existing strings (pass 8): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
                   "Time create existing strings (pass 9): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
                   "Time create existing strings (pass 10): \t\t%lldmsec \t\t(%0.4fmsec per string)\n"
           "Time create new strings: \t\t\t%lldmsec  \t(%0.4fmsec per string)\n"
           "Time read new strings:\t\t\t\t%lldmsec \t\t(%0.4fmsec per string)\n"
           "timer_strings_fullscan:\t%lldmsec\n"
           "timer_strings_fullscan_read:%lldmsec\n"
           "Time find string non existing:\t\t%lldmsec \t(%0.4fmsec per string)\n"

           "Time find string existing (new):\t\t%lldmsec \t(%0.4fmsec per string)\n"
           "Time find string existing (pass 1):\t\t%lldmsec \t(%0.4fmsec per string)\n"
           "Time find string existing (pass 2):\t\t%lldmsec \t(%0.4fmsec per string)\n"
           "Time find string existing (pass 2):\t\t%lldmsec \t(%0.4fmsec per string)\n",

            gs_timer_diff_ms(&timer_nodes_created),
            gs_timer_diff_ms(&timer_strings_existing_created_1),
            gs_timer_diff_ms(&timer_strings_existing_created_1) / (float) num_strings,
            gs_timer_diff_ms(&timer_strings_existing_created_2),
            gs_timer_diff_ms(&timer_strings_existing_created_2) / (float) num_strings,
            gs_timer_diff_ms(&timer_strings_existing_created_3),
            gs_timer_diff_ms(&timer_strings_existing_created_3) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_existing_created_4),
           gs_timer_diff_ms(&timer_strings_existing_created_4) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_existing_created_5),
           gs_timer_diff_ms(&timer_strings_existing_created_5) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_existing_created_6),
           gs_timer_diff_ms(&timer_strings_existing_created_6) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_existing_created_7),
           gs_timer_diff_ms(&timer_strings_existing_created_7) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_existing_created_8),
           gs_timer_diff_ms(&timer_strings_existing_created_8) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_existing_created_9),
           gs_timer_diff_ms(&timer_strings_existing_created_9) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_existing_created_10),
           gs_timer_diff_ms(&timer_strings_existing_created_10) / (float) num_strings,
            gs_timer_diff_ms(&timer_strings_non_existing_created),
            gs_timer_diff_ms(&timer_strings_non_existing_created) / (float) num_new_strings,
            gs_timer_diff_ms(&timer_strings_read),
            gs_timer_diff_ms(&timer_strings_read) / (float) num_new_strings,
            gs_timer_diff_ms(&timer_strings_fullscan),
            gs_timer_diff_ms(&timer_strings_fullscan_read),
            gs_timer_diff_ms(&timer_strings_find_non_existing),
            gs_timer_diff_ms(&timer_strings_find_non_existing) / (float) num_unknown_strings,


            gs_timer_diff_ms(&timer_strings_find_existing_new),
            gs_timer_diff_ms(&timer_strings_find_existing_new) / (float) num_new_strings,
           gs_timer_diff_ms(&timer_strings_find_existing_1),
           gs_timer_diff_ms(&timer_strings_find_existing_1) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_find_existing_2),
           gs_timer_diff_ms(&timer_strings_find_existing_2) / (float) num_strings,
           gs_timer_diff_ms(&timer_strings_find_existing_3),
           gs_timer_diff_ms(&timer_strings_find_existing_3) / (float) num_strings
    );

    for (int i = 0; i < 100; i++) {
        gs_timer_start(&timer_strings_find_existing_3);
        database_string_find(&num_strings_found, &found_str_ids, NULL, database, (char **) strings, num_strings);
        gs_timer_stop(&timer_strings_find_existing_3);

        printf("%lld;%0.4f\n",
               gs_timer_diff_ms(&timer_strings_find_existing_3),
               gs_timer_diff_ms(&timer_strings_find_existing_3) / (float) num_strings);
    }


    free (node_records);
    free (strings);
    free (strings_added);
    free (strings_added_strs);
    free (all_string_ids);
    free (loaded_strings);
    free (unknown_strings);
    free (found_str_ids);
    free (strings_found);

    return 0;
}