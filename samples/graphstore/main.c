
#include <storage/database.h>
#include <storage/timespan.h>

int main() {
    int num_new_nodes = 1;

    gs_timer_t timer_nodes_created, timer_strings_created, timer_strings_read, timer_strings_fullscan, timer_strings_fullscan_read, timer_strings_find;

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



    unsigned num_strings = 5;
    const char **strings = malloc(num_strings * sizeof(char *));
    for (int i = 0; i < num_strings; i++) {
        strings[i] = malloc(sizeof("Hr There!") + 1);
        strings[i] = "Hr There!";
    }

    gs_status_t status_add_str;
    size_t num_created_strings;

    gs_timer_start(&timer_strings_created);
    string_id_t *strings_added = database_string_create(&num_created_strings, &status_add_str, database, strings, num_strings, string_create_create_force);
    gs_timer_stop(&timer_strings_created);

    gs_timer_start(&timer_strings_read);
    char **strings_added_strs = database_string_read(NULL, database, strings_added, num_created_strings);
    gs_timer_start(&timer_strings_read);

    for (int i = 0; i < num_created_strings; i++) {
             printf("Created in db: '%lld: %s'\n", strings_added[i], strings_added_strs[i]);
    }

    size_t num_strings_in_db;
    gs_timer_start(&timer_strings_fullscan);
    string_id_t *all_string_ids = database_string_fullscan(&num_strings_in_db, NULL, database);
    gs_timer_start(&timer_strings_fullscan);

    gs_timer_start(&timer_strings_fullscan_read);
    char **loaded_strings = database_string_read(NULL, database, all_string_ids, num_strings_in_db);
    gs_timer_start(&timer_strings_fullscan_read);

    for (int i = 0; i < num_strings_in_db; i++) {
        printf("Load from db: '%lld: %s'\n", all_string_ids[i], loaded_strings[i]);
    }

    size_t num_strings_found = 0;
    char **needles = malloc (3 * sizeof(char **));
    needles[0] = malloc(strlen("Mouse") + 1);
    needles[1] = malloc(strlen("Terminator") + 1);
    needles[2] = malloc(strlen("Dog") + 1);
    strcpy(needles[0], "Mouse");
    strcpy(needles[1], "Terminator");
    strcpy(needles[2], "Dog");

    gs_timer_start(&timer_strings_find);
    string_id_t *found_str_ids;
    database_string_find(&num_strings_found, &found_str_ids, NULL, database, needles, 3);
    gs_timer_start(&timer_strings_find);

    char **strings_found = database_string_read(NULL, database, found_str_ids, num_strings_found);

    for (int i = 0; i < num_strings_found; i++) {
        printf("Load from db: '%lld: %s'\n", found_str_ids[i], strings_found[i]);
    }

    printf("\n-------------------------------------------\n"
           "timer_nodes_created:\t%lldmsec\n"
           "timer_strings_created:\t%lldmsec\n"
           "timer_strings_read:\t%lldmsec\n"
           "timer_strings_fullscan:\t%lldmsec\n"
           "timer_strings_fullscan_read:\t%lldmsec\n"
           "timer_strings_find:\t%lldmsec\n",
            gs_timer_diff_ms(&timer_nodes_created),
           gs_timer_diff_ms(&timer_strings_created),
           gs_timer_diff_ms(&timer_strings_read),
           gs_timer_diff_ms(&timer_strings_fullscan),
           gs_timer_diff_ms(&timer_strings_fullscan_read),
           gs_timer_diff_ms(&timer_strings_find));





    return 0;
}