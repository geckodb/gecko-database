
#include <storage/database.h>
#include <storage/timespan.h>

int main() {
    int num_new_nodes = 10;

    database_t *database;
    database_open(&database, "/Users/marcus/temp/db");
    gs_status_t status;
    timespan_t lifetime = { .begin = 0, .end = 200};
    node_id_t *node_records = database_nodes_create(&status, database, num_new_nodes, &lifetime);

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





    return 0;
}