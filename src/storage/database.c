#include <storage/database.h>
#include <storage/files.h>
#include <storage/dirs.h>
#include <apr_strings.h>

#define DB_NODES_FILE "nodes.db"
#define DB_EDGES_FILE "edges.db"
#define DB_STRINGPOOL_FILE "strings.db"

typedef struct database_t
{
    apr_pool_t         *apr_pool;
    char               *dbpath;
    FILE               *edges,
                       *nodes,
                       *string_pool,
                       *snapshots;
    const char         *nodes_db_path,
                       *edges_db_path,
                       *string_pool_path;
    gs_spinlock_t       spinlock;

} database_t;

typedef struct nodes_header_t
{
    node_id_t           next_node_id;
    node_id_t           capacity;
} nodes_header_t;

typedef struct instant_t
{
    timestamp_t         client_start_time,
                        system_start_time;
} instant_t;

typedef struct node_t
{
    struct {
        short           in_use            : 1;
        short           has_next_version  : 1;
        short           is_tombstone      : 1;
        short           has_prop          : 1;
        short           has_edge_in       : 1;
        short           has_edge_out      : 1;
    }                   flags;
    prop_id_t           next_prop;
    edge_id_t           next_edge_in;
    edge_id_t           next_edge_out;
    instant_t           start_time;
    node_id_t           next_version;
} node_t;

typedef struct edges_header_t
{
    edge_id_t           next_edge_id;
    edge_id_t           capacity;
} edges_header_t;

typedef struct edges_t
{
    struct {
        short           in_use            : 1;
        short           is_final_version  : 1;
    }                   flags;
    node_id_t           head_node,
                        tail_node;
    instant_t           start_time;
    prop_id_t           next_prop;
} edges_t;

typedef struct pool_header_t
{
    size_t              size_byte,
                        capacity_byte;
    size_t              slot_num,
                        slot_capacity;
} pool_header_t;

typedef struct pool_slot_t
{
    size_t              length;
    size_t              offset;
} pool_slot_t;

static inline gs_status_t db_exists(database_t *database);
static inline gs_status_t db_read_node_header(nodes_header_t *header, database_t *database);
static inline gs_status_t db_update_node_header(database_t *database, const nodes_header_t *header);

static inline gs_status_t create_db(database_t *database);
static inline gs_status_t create_nodes_db(database_t *database);
static inline gs_status_t create_edges_db(database_t *database);
static inline gs_status_t create_stringpool_db(database_t *database);
static inline gs_status_t open_db(database_t *database);
static inline gs_status_t open_nodes_db(database_t *database);
static inline gs_status_t open_edges_db(database_t *database);
static inline gs_status_t open_stringpool_db(database_t *database);

gs_status_t database_open(database_t **db, const char *dbpath)
{
    gs_status_t status;
    database_t *database;

    apr_initialize();
    database = GS_REQUIRE_MALLOC(sizeof(database_t));
    apr_pool_create(&database->apr_pool, NULL);
    database->dbpath = apr_pstrdup(database->apr_pool, dbpath);
    gs_spinlock_create(&database->spinlock);

    if ((status = dirs_add_filename(&database->nodes_db_path, database->dbpath, DB_NODES_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if ((status = dirs_add_filename(&database->edges_db_path, database->dbpath, DB_EDGES_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if ((status = dirs_add_filename(&database->string_pool_path, database->dbpath, DB_STRINGPOOL_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if ((status = db_exists(database)) == GS_FALSE) {
        status = create_db(database);
    } else {
        status = open_db(database);
    }

    *db = database;

    return status;
}

node_id_t *database_create_nodes(database_t *db, size_t num_nodes, timestamp_t client_start_time)
{
    gs_status_t status;
    nodes_header_t header;

    node_id_t *result = GS_REQUIRE_MALLOC(num_nodes * sizeof(node_id_t));

    // Acquire exclusive access to the database file
    gs_spinlock_lock(&db->spinlock);


    db_read_node_header(&header, db);
    if (header.next_node_id + num_nodes > header.capacity) {
        // resize
        size_t expand_size = 1.7f * (1 + num_nodes);
        fseek(db->nodes, 0, SEEK_END);

        node_t empty = {
            .flags.in_use = FALSE
        };

        for (int i = 0; i < expand_size; i++) {
            if (fwrite(&empty, sizeof (node_t), 1, db->nodes) != 1) {
                // Something went wrong during expansion. No changes are written to the database, since the
                // header is not updated (although the database file now is grown beyond the size indicated
                // be the capacity value).
                gs_spinlock_unlock(&db->spinlock);
                return NULL;
            }
        }

        header.capacity += expand_size;
    }

    fseek(db->nodes, sizeof(nodes_header_t) + header.next_node_id  * sizeof(node_t), SEEK_SET);
    node_t node = {
        .start_time = {
            .client_start_time = client_start_time,
            .system_start_time = (timestamp_t) time(NULL)
        },
            .flags.in_use           = TRUE,
            .flags.has_next_version = FALSE,
            .flags.is_tombstone     = FALSE,
            .flags.has_prop         = FALSE,
            .flags.has_edge_in      = FALSE,
            .flags.has_edge_out     = FALSE
    };
    for (int i = 0; i < num_nodes; i++) {
        if (fwrite(&node, sizeof(node_t), 1, db->nodes) != 1) {
            // Something went wront during write. Reset these changes by not updating the header (i.e., changes writen
            // so far are overwritten next time), release lock and notify about failure
            gs_spinlock_unlock(&db->spinlock);
            return NULL;
        };
        result[i] = header.next_node_id++;
    }

    // Update header in order to store that new nodes were written
    unsigned max_retry = 100;
    do {
        status = db_update_node_header(db, &header);
    } while (status != GS_SUCCESS && max_retry--);

    // Release exclusive access to the database file
    gs_spinlock_unlock(&db->spinlock);

    if (status != GS_SUCCESS) {
        // In this case, we lost the last updates since they will be overwritten next time
        perror("Unable to update database to latest changes");
        return NULL;
    }

    return result;
}

gs_status_t database_close(database_t *db)
{
    fclose(db->nodes);
    fclose(db->edges);
    fclose(db->string_pool);
    apr_pool_destroy(db->apr_pool);
    free (db);
    return GS_SUCCESS;
}

static inline gs_status_t db_exists(database_t *database)
{
    return (files_exists(database->edges_db_path) && files_exists(database->nodes_db_path) &&
            files_exists(database->string_pool_path)) ? GS_TRUE : GS_FALSE;
}

static inline gs_status_t db_read_node_header(nodes_header_t *header, database_t *database)
{
    fseek(database->nodes, 0, SEEK_SET);
    return (fread(header, sizeof(nodes_header_t), 1, database->nodes) == 1 ? GS_SUCCESS : GS_FAILED);
}

static inline gs_status_t db_update_node_header(database_t *database, const nodes_header_t *header)
{
    fseek(database->nodes, 0, SEEK_SET);
    return (fwrite(header, sizeof(nodes_header_t), 1, database->nodes) == 1 ? GS_SUCCESS : GS_FAILED);
}

static inline gs_status_t create_db(database_t *database)
{
    if (create_nodes_db(database) != GS_SUCCESS)
        return GS_FAILED;

    if (create_edges_db(database) != GS_SUCCESS)
        return GS_FAILED;

    if (create_stringpool_db(database) != GS_SUCCESS)
        return GS_FAILED;

    return GS_SUCCESS;
}

static inline gs_status_t create_nodes_db(database_t *database)
{
    nodes_header_t header = {
        .next_node_id = 0,
        .capacity     = DB_NODE_DEFAULT_CAPACITY
    };
    node_t node = {
        .flags.in_use = FALSE
    };

    if ((database->nodes = fopen(database->nodes_db_path, "wb")) == NULL)
        return GS_FAILED;

    if (fwrite(&header, sizeof (nodes_header_t), 1, database->nodes) != 1)
        return GS_FAILED;

    for (int i = 0; i < header.capacity; i++) {
        if (fwrite(&node, sizeof (node_t), 1, database->nodes) != 1)
            return GS_FAILED;
    }

    return GS_SUCCESS;
}

static inline gs_status_t create_edges_db(database_t *database)
{
    edges_header_t header = {
        .next_edge_id = 0,
        .capacity     = DB_EDGE_DEFAULT_CAPACITY
    };
    edges_t edge = {
        .flags.in_use = FALSE
    };

    if ((database->edges = fopen(database->edges_db_path, "wb")) == NULL)
        return GS_FAILED;

    if (fwrite(&header, sizeof (edges_header_t), 1, database->edges) != 1)
        return GS_FAILED;

    for (int i = 0; i < header.capacity; i++) {
        if (fwrite(&edge, sizeof(edges_t), 1, database->edges) != 1)
            return GS_FAILED;
    }

    return GS_SUCCESS;
}

static inline gs_status_t create_stringpool_db(database_t *database)
{
    pool_header_t header = {
        .size_byte     = 0,
        .capacity_byte = DB_STRING_POOL_CAPACITY_BYTE,
        .slot_num      = 0,
        .slot_capacity = DB_STRING_POOL_SLOT_CAPACITY
    };

    pool_slot_t slot = {
        .offset        = 0,
        .length        = 0
    };

    if ((database->string_pool = fopen(database->string_pool_path, "wb")) == NULL)
        return GS_FAILED;

    if (fwrite(&header, sizeof (pool_header_t), 1, database->string_pool) != 1)
        return GS_FAILED;

    for (int i = 0; i < header.slot_capacity; i++) {
        if (fwrite(&slot, sizeof(pool_slot_t), 1, database->string_pool) != 1)
            return GS_FAILED;
    }

    for (int i = 0; i < header.capacity_byte; i++) {
        char empty = 0;
        if (fwrite(&empty, sizeof(char), 1, database->string_pool) != 1)
            return GS_FAILED;
    }

    return GS_SUCCESS;
}

static inline gs_status_t open_db(database_t *database)
{
    if (open_nodes_db(database) != GS_SUCCESS)
        return GS_FAILED;

    if (open_edges_db(database) != GS_SUCCESS)
        return GS_FAILED;

    if (open_stringpool_db(database) != GS_SUCCESS)
        return GS_FAILED;

    return GS_SUCCESS;
}

static inline gs_status_t open_nodes_db(database_t *database)
{
    if ((database->nodes = fopen(database->nodes_db_path, "r+b")) == NULL)
        return GS_FAILED;
    return GS_SUCCESS;
}

static inline gs_status_t open_edges_db(database_t *database)
{
    if ((database->edges = fopen(database->edges_db_path, "r+b")) == NULL)
        return GS_FAILED;
    return GS_SUCCESS;
}

static inline gs_status_t open_stringpool_db(database_t *database)
{
    if ((database->string_pool = fopen(database->string_pool_path, "r+b")) == NULL)
        return GS_FAILED;
    return GS_SUCCESS;
}