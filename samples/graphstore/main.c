
#include <storage/database.h>

int main() {

    database_t *database;
    database_open(&database, "/Users/marcus/temp/db");
    /*node_id_t *nodes =*/ database_create_nodes(database, 90, time(NULL));

    return 0;
}