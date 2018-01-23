#ifndef GECKO_DATABASE_H
#define GECKO_DATABASE_H

#include <gecko-commons/gecko-commons.h>
#include <storage/timespan.h>

#define DB_NODE_DEFAULT_CAPACITY         100
#define DB_EDGE_DEFAULT_CAPACITY        1000
#define DB_STRING_POOL_CAPACITY_BYTE    10485760
#define DB_STRING_POOL_SLOT_CAPACITY     100

/* forwarding */
typedef struct pool_t pool_t;

typedef uint64_t node_id_t;

typedef uint64_t prop_id_t;

typedef uint64_t edge_id_t;

typedef struct database_t database_t;

gs_status_t database_open(database_t **db, const char *dbpath);
node_id_t *database_create_nodes(database_t *db, size_t num_nodes, timestamp_t client_start_time);
gs_status_t database_close(database_t *db);


#endif
