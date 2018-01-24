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

typedef uint64_t offset_t;

typedef uint64_t node_slot_id_t;

typedef uint64_t prop_slot_id_t;

typedef uint64_t edge_slot_id_t;

typedef struct database_t database_t;

typedef struct __attribute__((__packed__)) database_node_t
{
    node_id_t           unique_id;
    struct {
        short           in_use            : 1;
        short           has_version_prev  : 1;
        short           has_version_next  : 1;
        short           has_prop          : 1;
        short           has_edge_in       : 1;
        short           has_edge_out      : 1;
    }                   flags;

    prop_slot_id_t      next_prop;
    edge_slot_id_t      next_edge_in;
    edge_slot_id_t      next_edge_out;

    timespan_t          lifetime;
    timestamp_t         creation_time;

    node_slot_id_t      version_prev;
    node_slot_id_t      version_next;
} database_node_t;

typedef enum data_type_e {
    data_type_int8, data_type_int16, data_type_int32, data_type_int64,
    data_type_uint8, data_type_uint16, data_type_uint32, data_type_uint64,
    data_type_float32, data_type_float64, data_type_string, data_type_void,
    data_type_timedate
} data_type_e;

typedef int8_t      gecko_int8_t;

typedef int16_t     gecko_int16_t;

typedef int32_t     gecko_int32_t;

typedef int64_t     gecko_int64_t;

typedef uint8_t     gecko_uint8_t;

typedef uint16_t    gecko_uint16_t;

typedef uint32_t    gecko_uint32_t;

typedef uint64_t    gecko_uint64_t;

typedef float       gecko_float32_t;

typedef double      gecko_float64_t;

typedef const char *gecko_string_t;

typedef uint64_t    gecko_timedate_t;

typedef struct value_t {
    data_type_e          type;
    union {
        gecko_int8_t     int8;
        gecko_int16_t    int16;
        gecko_int32_t    int32;
        gecko_int64_t    int64;
        gecko_uint8_t    uint8;
        gecko_uint16_t   uint16;
        gecko_uint32_t   uint32;
        gecko_uint64_t   uint64;
        gecko_float32_t  float32;
        gecko_float64_t  float64;
        gecko_string_t   string;
        gecko_timedate_t timedate;
    } data;
} value_t;

typedef enum prop_target_e {
    prop_target_node, prop_target_edge
} prop_target_e;

typedef struct target_t {
    prop_target_e       type;
    union {
        node_id_t       node_id;
        prop_id_t       prop_id;
    } ref;
} target_t;

typedef struct database_node_cursor_t database_node_cursor_t;

typedef struct database_node_version_cursor_t database_node_version_cursor_t;

gs_status_t database_open(database_t **db, const char *dbpath);
void        database_lock(database_t *db);
void        database_unlock(database_t *db);

node_id_t  *database_nodes_create(gs_status_t *result, database_t *db, size_t num_nodes, const timespan_t *lifetime);
gs_status_t database_nodes_alter_lifetime(database_t *db, node_id_t *nodes, size_t num_nodes, const timespan_t *lifetime);

node_id_t   database_nodes_last_id(database_t *db);
gs_status_t database_nodes_fullscan(database_node_cursor_t **cursor, database_t *db);

gs_status_t database_node_cursor_open(database_node_cursor_t *cursor);
gs_status_t database_node_cursor_next(database_node_cursor_t *cursor);
gs_status_t database_node_cursor_read(database_node_t *node, const database_node_cursor_t *cursor);
gs_status_t database_node_cursor_close(database_node_cursor_t *cursor);

// WARNING: node_version_cursors never acquire a lock on the node storage database. When this kind of cursor
// is used, make sure to lock/unlock the database
gs_status_t database_node_version_cursor_open(database_node_version_cursor_t **cursor, database_node_t *node, database_t *db);
gs_status_t database_node_version_cursor_next(database_node_version_cursor_t *cursor);
gs_status_t database_node_version_cursor_read(database_node_t *node, database_node_version_cursor_t *cursor);
gs_status_t database_node_version_cursor_close(database_node_version_cursor_t *cursor);


edge_id_t   database_create_edge(database_t *db, node_id_t head, node_id_t tail, timestamp_t client_start_time);

prop_id_t  *database_create_prop(database_t *db, const target_t *target, const char *key, const value_t *value,
                                 timestamp_t client_start_time);

gs_status_t database_close(database_t *db);


#endif
