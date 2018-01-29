#include <storage/database.h>
#include <storage/files.h>
#include <storage/dirs.h>
#include <apr_strings.h>

// The file containing node records and the per-node version chain
#define DB_NODES_FILE                "nodes.records"

// The file mapping a unique node id to its first version in the node file
#define DB_NODES_INDEX_FILE          "nodes.heads"


// The file containing property records and the per-property version chain
#define DB_PROPS_FILE                "props.records"

// The file mapping a unique prop id to its first version in the property file
#define DB_PROPS_INDEX_FILE          "props.heads"

// A file containing lists of props used to attach a set of properties to nodes and edges
#define DB_PROPS_LIST_FILE           "proplists.records"

// A file mapping a unique prop list id to its offset in the props list file
#define DB_PROPS_LIST_INDEX_FILE     "proplists.heads"


// A file containing variable-length strings
#define DB_STRINGPOOL_FILE           "strings.records"

// A file mapping a unique string identifier to its offset and legnth in the strings record file
#define DB_STRINGPOOL_INDEX_FILE     "strings.lookup"


#define DB_EDGES_FILE           "edges.records"


typedef struct database_t
{
    apr_pool_t         *apr_pool;
    char               *dbpath;
    FILE               *node_records,
                       *node_heads,

                       *prop_records,
                       *prop_heads,
                       *proplist_records,
                       *proplist_heads,

                       *string_records,
                       *string_lookup,

                       *edge_records;


    const char         *nodes_records_path,
                       *nodes_heads_path,

                       *prop_records_path,
                       *prop_heads_path,
                       *proplist_records_path,
                       *proplist_heads_path,

                       *string_records_path,
                       *string_lookup_path,

                       *edges_db_path;
    gs_spinlock_t       spinlock;

} database_t;

typedef struct __attribute__((__packed__)) nodes_header_t
{
    node_id_t                next_node_id;
    node_slot_id_t           next_node_slot_id;
    node_slot_id_t           capacity;
} node_records_header_t;

typedef struct __attribute__((__packed__)) string_records_header_t
{
    offset_t            capacity_in_byte,
                        size_in_byte;
    offset_t            write_offset;
} string_records_header_t;

typedef struct string_record_entry_header_t
{
    size_t              string_length;
} string_record_entry_header_t;


typedef struct __attribute__((__packed__)) string_lookup_header_t
{
    string_id_t         id_cursor;
    size_t              capacity;
} string_lookup_header_t;

typedef struct __attribute__((__packed__)) string_lookup_entry_t
{
    offset_t            string_offset;
    union {
        short           in_use : 1;
    } flags;
} string_lookup_entry_t;

typedef struct __attribute__((__packed__)) edges_header_t
{
    edge_slot_id_t           next_edge_id;
    edge_slot_id_t           capacity;
} edges_header_t;

typedef struct __attribute__((__packed__)) edges_t
{
    struct {
        short           in_use            : 1;
        short           is_final_version  : 1;
    }                   flags;
    node_slot_id_t             head_node,
                               tail_node;
    timestamp_t                start_time;
    prop_slot_id_t             next_prop;
} edges_t;

typedef struct __attribute__((__packed__)) node_index_header_t
{
    node_id_t           node_cursor;
} node_index_header_t;

typedef node_slot_id_t  node_index_entry_t;

typedef struct database_node_cursor_t
{
    database_t          *database;
    node_id_t            max_node_id,
                         current_node_id;
    bool                 open;
    bool                 first;
} database_node_cursor_t;

typedef struct database_node_version_cursor_t
{
    database_t          *database;
    database_node_t      node;
} database_node_version_cursor_t;

typedef struct __attribute__((__packed__)) props_header_t
{
    prop_id_t                next_props_id;
    prop_slot_id_t           next_props_slot_id;
    prop_slot_id_t           capacity;
} props_header_t;

static inline gs_status_t exists_db(database_t *database);

static inline gs_status_t unsafe_nodes_db_autoresize(database_t *db, node_records_header_t *header, size_t num_nodes);
static inline gs_status_t unsafe_read_node_records_header(node_records_header_t *header, database_t *database);
static inline gs_status_t unsafe_read_node_index_header(node_index_header_t *header, database_t *database);
static inline gs_status_t unsafe_update_node_records_header(database_t *database, const node_records_header_t *header);
static inline gs_status_t unsafe_update_node_index_header(database_t *database, const node_index_header_t *header);
static inline node_slot_id_t  unsafe_read_node(database_node_t *node, node_id_t node_id, database_t *database);
static inline node_slot_id_t  unsafe_node_last_version(database_node_t *last, const database_node_t *start,
                                                       node_slot_id_t start_slot, database_t *db);
static inline gs_status_t unsafe_node_new_version(node_slot_id_t *cpy_slot_id, database_node_t *cpy,
                                                  database_node_t *original, database_t *db,
                                                  size_t approx_num_new_nodes, node_slot_id_t original_slot);
static inline gs_status_t unsafe_node_adjust_lifetime(database_node_t *node, node_slot_id_t slot_id,
                                                      database_t *db, const timespan_t *lifetime);
static inline gs_status_t unsafe_read_string_records_header(string_records_header_t *header, database_t *db);
static inline gs_status_t unsafe_read_string_lookup_header(string_lookup_header_t *header, database_t *db);
static inline gs_status_t unsafe_update_string_records_header(database_t *database, const string_records_header_t *header);
static inline gs_status_t unsafe_update_string_index_header(database_t *database,
                                                            const string_lookup_header_t *header);

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

    if ((status = dirs_add_filename(&database->nodes_records_path, database->dbpath, DB_NODES_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if ((status = dirs_add_filename(&database->nodes_heads_path, database->dbpath, DB_NODES_INDEX_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if ((status = dirs_add_filename(&database->edges_db_path, database->dbpath, DB_EDGES_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if ((status = dirs_add_filename(&database->string_records_path, database->dbpath, DB_STRINGPOOL_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if ((status = dirs_add_filename(&database->string_lookup_path, database->dbpath, DB_STRINGPOOL_INDEX_FILE,
                                    database->apr_pool)) != GS_SUCCESS)
        return status;

    if (exists_db(database) == GS_FALSE) {
        if (create_db(database) == GS_SUCCESS && database_close(database) == GS_SUCCESS) {
            return database_open(db, dbpath);
        } else return GS_FAILED;
    }

    status = open_db(database);

    *db = database;

    return status;
}

// Seek to slot in node records file
#define NODE_RECORDS_FILE_SEEK(db, slot_id)                                                                            \
    fseek(db->node_records, sizeof(node_records_header_t) + slot_id  * sizeof(database_node_t), SEEK_SET);

// Seek to index slot for particular node id
#define NODE_INDEX_FILE_SEEK(db, node_id)                                                                              \
    fseek(db->node_heads, sizeof(node_index_header_t) + node_id * sizeof(node_index_entry_t), SEEK_SET);

node_id_t *database_nodes_create(gs_status_t *result, database_t *db, size_t num_nodes, const timespan_t *lifetime)
{
    node_records_header_t records_header;
    node_index_header_t   index_header;

    // For rollback operations
    node_id_t             node_record_next_id_backup;
    node_slot_id_t        node_record_next_slot_id_backup;
    node_id_t             node_index_cursor_backup;

    node_id_t *retval = GS_REQUIRE_MALLOC(num_nodes * sizeof(node_id_t));

    database_lock(db);

    if (unsafe_read_node_records_header(&records_header, db) != GS_SUCCESS ||
            unsafe_read_node_index_header(&index_header, db) != GS_SUCCESS) {
        GS_OPTIONAL(result != NULL, *result = GS_LOAD_FAILED)
        free (retval);
        return NULL;
    }

    // Store old information from the header in case writes to the storage or index fails
    node_record_next_id_backup = records_header.next_node_id;
    node_record_next_slot_id_backup = records_header.next_node_slot_id;
    node_index_cursor_backup = index_header.node_cursor;

    assert(records_header.next_node_id == index_header.node_cursor);

    if (unsafe_nodes_db_autoresize(db, &records_header, num_nodes) != GS_SUCCESS) {
        // Something went wrong during expansion. No changes are written to the database, since the
        // records_header is not updated (although the database file now is grown beyond the size indicated
        // be the capacity_in_byte value).
        database_unlock(db);
        return NULL;
    }

    // Seek to next free slot in node records file
    NODE_RECORDS_FILE_SEEK(db, records_header.next_node_slot_id);

    // Seek to next index slot in the node index file
    NODE_INDEX_FILE_SEEK(db, index_header.node_cursor);

    // New note to be inserted. The node is setup with client and system time but blank content, i.e., no properties and
    // no references to other version. However, the node begins its existing at client/system time and has currently
    // and infinite lifetime.
    database_node_t node = {
        .lifetime               = *lifetime,
        .flags.in_use           = TRUE,
        .flags.has_version_prev = FALSE,
        .flags.has_version_next = FALSE,
        .flags.has_prop         = FALSE,
        .flags.has_edge_in      = FALSE,
        .flags.has_edge_out     = FALSE
    };

    // Store new notes to the nodes record file
    for (int i = 0; i < num_nodes; i++) {
        // Set unique node id and system creation time
        node.unique_id = records_header.next_node_id;
        node.creation_time = GS_SYSTEM_TIME_MS();

        // Get the current write first_slot which will be used in the index to enable an efficient mapping from
        // unique node id to its first version
        node_index_entry_t first_slot = records_header.next_node_slot_id++;

        // Write node to store
        if (fwrite(&node, sizeof(database_node_t), 1, db->node_records) != 1) {
            // Something went wront during write. Reset these changes by not updating the records_header (i.e., changes writen
            // so far are overwritten next time), release lock and notify about failure
            database_unlock(db);
            GS_OPTIONAL(result != NULL, *result = GS_WRITE_FAILED)
            free (retval);
            return NULL;
        };

        // Register first_slot of this node object in the index file such that the unique node id (i.e., the i-th
        // element) in the index file maps to the first_slot of the first node in the storage file.
        if (fwrite(&first_slot, sizeof(node_index_entry_t), 1, db->node_heads) != 1) {
            // In case this write fails, the index to the new first_slot of some added nodes (these before this write fails)
            // were written. However, since the index header is not updated, these changes will be overwritten the next
            // time. Thus, no rollback is needed.
            database_unlock(db);
            GS_OPTIONAL(result != NULL, *result = GS_WRITE_FAILED)
            free (retval);
            return NULL;
        };
        retval[i] = records_header.next_node_id++;
        index_header.node_cursor = records_header.next_node_id;
    }

    // Write down everything which is in the output buffer
    fflush(db->node_records);
    fflush(db->node_heads);

    // Update both the records header and the index header to register the changes finally.
    unsigned max_retry;
    gs_status_t status_store_update, status_index_update;

    // Update records header in order to store that new node records were written. Give this 100 tries or fails.
    max_retry = 100;
    do {
        status_store_update = unsafe_update_node_records_header(db, &records_header);
    } while (status_store_update != GS_SUCCESS && max_retry--);

    // Update index header in order to store that new nod _records were written. Give this 100 tries or fails.
    max_retry = 100;
    do {
        status_index_update = unsafe_update_node_index_header(db, &index_header);
    } while (status_index_update != GS_SUCCESS && max_retry--);

    if (status_store_update != GS_SUCCESS) {
        // Updating the store header fails for some reasons. Depending on the outcome of the index write this
        // may lead to an serious issue
        if (status_index_update != GS_SUCCESS) {
            // Both update request fail which means that data might be written to the store and the index but
            // their registration (in the header) is missing. Therefore, these changes will be overwritten the next
            // time. To sum up, this case is an database insert failure without side effects.

            // Nothing to do but to release the lock and notify about failure.

            GS_OPTIONAL(result != NULL, *result = GS_WRITE_FAILED)
        } else {
            // The index is updated and now points to nodes that are not stored in the store. Try to rollback the
            // index update by resetting the index header.
            index_header.node_cursor = node_index_cursor_backup;

            // Update index header in order to store that new nod _records were written. Give this 100 tries or fails.
            max_retry = 100;
            do {
                status_index_update = unsafe_update_node_index_header(db, &index_header);
            } while (status_index_update != GS_SUCCESS && max_retry--);

            if (status_index_update == GS_SUCCESS) {
                // Rollback successful, flush buffers
                fflush(db->node_heads);
                GS_OPTIONAL(result != NULL, *result = GS_WRITE_FAILED)
            } else {
                // Rollback failed which means that the index is damaged and not repairable by only resetting the
                // index header.
                // TODO: Rebuild the index file and try to swap the damaged index file with the new one
                panic("Node index file '%s' was corrupted and cannot be repaired.", db->nodes_heads_path);
                GS_OPTIONAL(result != NULL, *result = GS_CORRUPTED)
            }
        }
        free (retval);
        database_unlock(db);
        return NULL;
    } else {
        // Updating the store header was successful. Depending on the index write outcome, there is maybe another issue
        if (status_index_update != GS_SUCCESS) {
            // The storage now contains registered nodes which are not receivable from the index. Even worse, since
            // the index is not updates, subsequent writes now will lead to an index mapping not to the new nodes
            // in that call but to the nodes that were written with this call. Try to rollback changes in the
            // storage file by resetting the header.

            records_header.next_node_id = node_record_next_id_backup;
            records_header.next_node_slot_id = node_record_next_slot_id_backup;

            max_retry = 100;
            do {
                status_store_update = unsafe_update_node_records_header(db, &records_header);
            } while (status_store_update != GS_SUCCESS && max_retry--);

            if (status_store_update == GS_SUCCESS) {
                // Rollback was successful
                fflush(db->node_records);
                GS_OPTIONAL(result != NULL, *result = GS_WRITE_FAILED)
            } else {
                // Rollback failed which means that the storage is damaged and not repairable by only resetting the
                // storage header.
                // TODO: Handling this failure case to avoid corruption of database
                panic("Node storage file '%s' was corrupted and cannot be repaired.", db->nodes_records_path);
                GS_OPTIONAL(result != NULL, *result = GS_CORRUPTED)
            }
            free (retval);
            database_unlock(db);
            return NULL;
        } else {
            // Both updates were performed successfully. Release the lock and notify about this success.
            database_unlock(db);
            GS_OPTIONAL(result != NULL, *result = GS_SUCCESS)
            return retval;
        }
    }
}

gs_status_t database_nodes_alter_lifetime(database_t *db, node_id_t *nodes, size_t num_nodes, const timespan_t *lifetime)
{
    database_node_t   node_first, node_last, new_node;
    node_slot_id_t    first_slot, last_slot, cpy_slot;
    node_id_t        *node_id;

    database_lock(db);

    while (num_nodes--) {
        node_id = nodes++;
        first_slot = unsafe_read_node(&node_first, *node_id, db);
        last_slot  = unsafe_node_last_version(&node_last, &node_first, first_slot, db);
        if (unsafe_node_new_version(&cpy_slot, &new_node, &node_last, db, num_nodes, last_slot) != GS_SUCCESS) {
            // Database file could not be resized although that is required. Thus, this operations fails
            // but eventual changes are overwritten by the next call since the header is not updated.
            // Unlock the database and notify about the failure.
            database_unlock(db);
            return GS_FAILED;
        }
        if (unsafe_node_adjust_lifetime(&new_node, cpy_slot, db, lifetime) != GS_SUCCESS) {
            database_unlock(db);
            return GS_FAILED;
        }
    }
    fflush(db->node_records);

    database_unlock(db);
    return GS_SUCCESS;
}

void database_lock(database_t *db)
{
    // Acquire exclusive access to the database file
    gs_spinlock_lock(&db->spinlock);
}

void database_unlock(database_t *db)
{
    // Release exclusive access to the database file
    gs_spinlock_unlock(&db->spinlock);
}

node_id_t database_nodes_last_id(database_t *db)
{
    node_records_header_t header;

    database_lock(db);
    unsafe_read_node_records_header(&header, db);
    database_unlock(db);

    return header.next_node_id;
}

gs_status_t database_nodes_fullscan(database_node_cursor_t **cursor, database_t *db)
{
    database_node_cursor_t *result = GS_REQUIRE_MALLOC(sizeof(database_node_cursor_t));
    result->current_node_id = 0;
    result->open            = FALSE;
    result->first           = TRUE;
    result->database        = db;
    result->max_node_id     = database_nodes_last_id(db);
    *cursor = result;
    return GS_SUCCESS;
}

gs_status_t database_node_cursor_open(database_node_cursor_t *cursor)
{
    if (cursor && !cursor->open) {
        database_lock(cursor->database);
        cursor->open = TRUE;
    }
    return cursor ? GS_SUCCESS : GS_ILLEGALARG;
}

gs_status_t database_node_cursor_next(database_node_cursor_t *cursor)
{
    cursor->current_node_id = cursor->current_node_id + (cursor->first ? 0 : 1);
    cursor->first = FALSE;
    return (cursor->current_node_id < cursor->max_node_id) ? GS_TRUE : GS_FALSE;
}

gs_status_t database_node_cursor_read(database_node_t *node, const database_node_cursor_t *cursor)
{
    bool valid = (node && cursor && cursor->open);
    if (valid) {
        unsafe_read_node(node, cursor->current_node_id, cursor->database);
    }
    return (valid ? GS_SUCCESS : GS_FAILED);
}

gs_status_t database_node_cursor_close(database_node_cursor_t *cursor)
{
    if (cursor && cursor->open) {
        database_unlock(cursor->database);
        free (cursor);
    }
    return cursor ? GS_SUCCESS : GS_ILLEGALARG;
}

gs_status_t database_node_version_cursor_open(database_node_version_cursor_t **result, database_node_t *node,
                                              database_t *db)
{
    database_node_version_cursor_t *cursor = GS_REQUIRE_MALLOC(sizeof(database_node_version_cursor_t));
    cursor->database = db;
    cursor->node = *node;
    *result = cursor;
    return GS_SUCCESS;
}

gs_status_t database_node_version_cursor_next(database_node_version_cursor_t *cursor)
{
    bool result = (cursor->node.flags.has_version_next ? GS_TRUE : GS_FALSE );
    NODE_RECORDS_FILE_SEEK(cursor->database, cursor->node.version_next);
    fread(&cursor->node, sizeof(database_node_t), 1, cursor->database->node_records);
    return result;
}

gs_status_t database_node_version_cursor_read(database_node_t *node, database_node_version_cursor_t *cursor)
{
    *node = cursor->node;
    return GS_SUCCESS;
}

gs_status_t database_node_version_cursor_close(database_node_version_cursor_t *cursor)
{
    free (cursor);
    return GS_SUCCESS;
}

// TODO: String create must install UNIQUE strings in the db file
string_id_t *database_string_create(size_t *num_created_strings, gs_status_t *status, database_t *db,
                                    const char **strings, size_t num_strings_to_import, string_create_mode_t mode)
{
    gs_status_t                 status_store_update, status_index_update;
    string_id_t                 backup_in_cursor;
    offset_t                    backup_write_offset;
    string_records_header_t     records_header;
    string_lookup_header_t      lookup_header;
    char                      **unique_strings_to_import;
    const char                 *string;
    string_id_t                *strings_found_ids;
    size_t                      result_size = 0;
    char                        empty;
    size_t                      required_size = 0;
    unsigned                    max_retry;

    // Deduplicate input strings
    gs_hashset_t hashset;
    hashset_create(&hashset, sizeof(char **), num_strings_to_import, gs_comp_func_str);
    for (size_t i = 0; i < num_strings_to_import; i++) {
        hashset_add(&hashset, (strings + i), 1, gs_comp_func_str);
    }
    const char **begin = (const char **) hashset_begin(&hashset);
    const char **end   = (const char **) hashset_end(&hashset);
    num_strings_to_import = (end - begin);
    unique_strings_to_import = (char **) hashset_begin(&hashset);

    // Determine new strings that are not already stored in the database
    size_t num_strings_found;
    size_t num_new_strings;
    char **new_strings = NULL;
    if (database_string_find(&num_strings_found, &strings_found_ids, &new_strings, db, unique_strings_to_import,
                             num_strings_to_import)
        != GS_SUCCESS) {
        return GS_FAILED;
    }

    num_new_strings = num_strings_to_import - num_strings_found;
    if (num_new_strings > 0)
    {
        // At least one string from the input is unknown in the database. Register only new strings in the database

        strings_found_ids = GS_REQUIRE_MALLOC(num_strings_to_import * sizeof(string_id_t));

        database_lock(db);

        if ((unsafe_read_string_records_header(&records_header, db) != GS_SUCCESS) ||
            unsafe_read_string_lookup_header(&lookup_header, db) != GS_SUCCESS) {
            // No access to string records or lookup header, release lock and return
            GS_OPTIONAL(status != NULL, *status = GS_READ_FAILED);
            free(strings_found_ids);
            database_unlock(db);
            return NULL;
        };

        // Backup header information for eventually rollback
        backup_in_cursor = lookup_header.id_cursor;
        backup_write_offset = records_header.write_offset;

        // Calculate required space to store all strings in the file. Note that strings in the string storage file
        // are not null-terminated but their length is defined in the lookup file
        for (size_t i = 0; i < num_strings_to_import; i++) {
            required_size += strlen(*(unique_strings_to_import + i));
        }

        // Eventually resize the lookup file
        if (lookup_header.id_cursor + num_strings_to_import > lookup_header.capacity) {
            lookup_header.capacity = 1.7 * (lookup_header.capacity + num_strings_to_import);
            fseek(db->string_lookup,
                  sizeof(string_lookup_header_t) + lookup_header.id_cursor * sizeof(string_lookup_entry_t), SEEK_SET);

            string_lookup_entry_t empty_entry = {
                    .flags.in_use = FALSE
            };

            for (int i = 0; i < num_strings_to_import; i++) {
                if (fwrite(&empty_entry, sizeof(string_lookup_entry_t), 1, db->string_lookup) != 1) {
                    // Expanding the lookup file failed. Notify caller about that failure and release lock
                    GS_OPTIONAL(status != NULL, *status = GS_FAILED);
                    free(strings_found_ids);
                    database_unlock(db);
                    return NULL;
                }
            }
        }

        // Eventually resize the file
        records_header.size_in_byte += required_size;
        if (records_header.size_in_byte > records_header.capacity_in_byte) {
            records_header.capacity_in_byte = 1.7f * records_header.size_in_byte;
            fseek(db->string_records, sizeof(string_records_header_t) + records_header.capacity_in_byte, SEEK_SET);
            if (fwrite(&empty, sizeof(char), 1, db->string_records) != 1) {
                // String records file cannot be resized to the required size. Notify caller about failure
                // and release look.
                GS_OPTIONAL(status != NULL, *status = GS_FAILED);
                free(strings_found_ids);
                database_unlock(db);
                return NULL;
            }

            fflush(db->string_records);
        }

        switch (mode) {
            case string_create_create_force:
                while (num_strings_to_import--) {
                    string = *unique_strings_to_import++;

                    // Write string into string string store
                    fseek(db->string_records, records_header.write_offset, SEEK_SET);

                    string_record_entry_header_t entry_header = {
                            .string_length = strlen(string)
                    };

                    // First write the header for that entry which contains the string length
                    if (fwrite(&entry_header, sizeof(string_record_entry_header_t), 1, db->string_records) != 1) {
                        // Writing the last string header into the records file failed. Until this point, some string
                        // from 'unique_strings_to_import' argument may be written. Since the header is not updated, these changes
                        // will be overwritten next time. Notify caller about failure and release lock.
                        GS_OPTIONAL(status != NULL, *status = GS_FAILED);
                        free(strings_found_ids);
                        database_unlock(db);
                        return NULL;
                    }

                    if (fwrite(string, strlen(string), 1, db->string_records) != 1) {
                        // Writing the last string into the records file failed. Until this point, some string
                        // from 'unique_strings_to_import' argument may be written. Since the header is not updated, these changes
                        // will be overwritten next time. Notify caller about failure and release lock.
                        GS_OPTIONAL(status != NULL, *status = GS_FAILED);
                        free(strings_found_ids);
                        database_unlock(db);
                        return NULL;
                    }

                    // Add the newly added string to the string index
                    string_lookup_entry_t entry = {
                            .flags.in_use = TRUE,
                            .string_offset = records_header.write_offset
                    };

                    fseek(db->string_lookup, sizeof(string_lookup_header_t) +
                                             lookup_header.id_cursor * sizeof(string_lookup_entry_t), SEEK_SET);

                    if (fwrite(&entry, sizeof(string_lookup_entry_t), 1, db->string_lookup) != 1) {
                        // Write into index failed. Until here, the string is actually written to the store
                        // but since neither the header of the store nor the index header are updated, no
                        // data corruption will occur. Release lock and notify caller about failure.
                        GS_OPTIONAL(status != NULL, *status = GS_FAILED);
                        free(strings_found_ids);
                        database_unlock(db);
                        return NULL;
                    }

                    // Return the string identifier matching this string. Note that this returns directly the
                    // offset in the storage file such that once one have a string id, no access to the lookup file
                    // is required.
                    strings_found_ids[result_size++] = records_header.write_offset;

                    // Update write offset such that the position for the next string to write is known
                    records_header.write_offset += sizeof(string_record_entry_header_t) + strlen(string);

                    // Update the id cursor for the next input
                    lookup_header.id_cursor++;
                }
                break;
            case string_create_create_noforce: panic("Not implemented '%d'", mode);
                break;
            default: panic("Unknown string mode '%d'", mode);
        }

        fflush(db->string_records);
        fflush(db->string_lookup);

        // Both the strings as well as the index entries are written. Now store these changes into the storage
        // resp. index file

        // Update records header in order to store that new node records were written. Give this 100 tries or fails.
        max_retry = 100;
        do {
            status_store_update = unsafe_update_string_records_header(db, &records_header);
        } while (status_store_update != GS_SUCCESS && max_retry--);

        // Update index header in order add the new string into the index. Give this 100 tries or fails.
        max_retry = 100;
        do {
            status_index_update = unsafe_update_string_index_header(db, &lookup_header);
        } while (status_index_update != GS_SUCCESS && max_retry--);

        // Handle file write statuses
        if (status_store_update != GS_SUCCESS) {
            // Updating the store header failed. Depending on the outcome of the index update, this might
            // leader to problems.
            if (status_index_update != GS_SUCCESS) {
                // Both updates fail. Since neither the store nor the index header is updated, the stored strings so far
                // will be overwritten by the next call of that function. Notify the caller about this reject and
                // release lock.
                GS_OPTIONAL(status != NULL, *status = GS_WRITE_FAILED);
            } else {
                // In this case, the index was updated but the store was not. Without repair, the index now will
                // point to offsets in the store file which are invalid. Solution is rollback of the index update.

                // Try rollback to index update. Give this 100 tries or fails.
                max_retry = 100;
                lookup_header.id_cursor = backup_in_cursor;
                do {
                    status_index_update = unsafe_update_string_index_header(db, &lookup_header);
                } while (status_index_update != GS_SUCCESS && max_retry--);

                if (status_index_update == GS_SUCCESS) {
                    // Rollback successful, flush buffers
                    fflush(db->string_lookup);
                    GS_OPTIONAL(status != NULL, *status = GS_WRITE_FAILED)
                } else {
                    // Rollback failed which means that the index is damaged and not repairable by only resetting the
                    // storage header.
                    // TODO: Handling this failure case to avoid corruption of database
                    panic("String lookup index file '%s' was corrupted and cannot be repaired.",
                          db->string_lookup_path);
                    GS_OPTIONAL(status != NULL, *status = GS_CORRUPTED)
                }
            }

            // In this case, at least one write fails. Cleanup result, release lock and return to caller
            free(strings_found_ids);
            database_unlock(db);
            return NULL;
        } else {
            // Sring store update was successful
            if (status_index_update != GS_SUCCESS) {
                // Index update was not successful. The string store now contains strings that are not reachable by the
                // index. Solution is rollback of changes to the store

                // Try rollback
                records_header.write_offset = backup_write_offset;

                // Update index header in order add the new string into the index. Give this 100 tries or fails.
                max_retry = 100;
                do {
                    status_index_update = unsafe_update_string_index_header(db, &lookup_header);
                } while (status_index_update != GS_SUCCESS && max_retry--);

                if (status_index_update == GS_SUCCESS) {
                    // Rollback was successful
                    fflush(db->string_records);
                    GS_OPTIONAL(status != NULL, *status = GS_FAILED)
                } else {
                    // Rollback was not successful. In this case data was written to the store and is not reachable
                    // by the index. However, there is no data corruption but storage waste. Notify the caller about this.
                    GS_OPTIONAL(status != NULL, *status = GS_WASTE)
                }

                // Cleanup and return a failure to the caller
                free(strings_found_ids);
                database_unlock(db);
                return NULL;

            } else {
                // Both updates were performed successfully. Release the lock and notify about this success.
                GS_OPTIONAL(status != NULL, *status = GS_SUCCESS)
                *num_created_strings = result_size;
                database_unlock(db);
                return strings_found_ids;
            }
        }
    } else {
        // All strings to be created were already present in the database
        *num_created_strings = num_strings_to_import;
        return strings_found_ids;
    }
}

static inline int comp_str(const void *lhs, const void *rhs)
{
    return strcmp(*(const char **) lhs, *(const char **) rhs);
}

gs_status_t database_string_find(size_t *num_strings_found, string_id_t **strings_found, char ***strings_not_found,
                                 database_t *db, char **strings, size_t num_strings)
{
    gs_status_t status;
    string_id_t *db_string_ids;
    string_id_t *db_strings_match_ids;
    size_t       num_strings_in_db;
    size_t       result_size;
    char       **strings_found_result;
    char       **strings_not_found_result;

    if ((db_string_ids = database_string_fullscan(&num_strings_in_db, &status, db)) && status != GS_SUCCESS) {
        return GS_FAILED;
    }

    qsort(strings, num_strings, sizeof(char **), comp_str);
    result_size = 0;

    if (strings_found) {
        db_strings_match_ids = GS_REQUIRE_MALLOC(num_strings * sizeof(string_id_t));
    }

    if (strings_not_found) {
        strings_found_result = GS_REQUIRE_MALLOC(num_strings * sizeof(char *));
    }


    char **db_string_entry = GS_REQUIRE_MALLOC(sizeof(char *));

    for (size_t  i = 0; i < num_strings_in_db; i++) {
        string_id_t *id_cursor = db_string_ids + i;
        char **value = database_string_read(&status, db, id_cursor, 1);

        if (status != GS_SUCCESS) {
            free (db_string_ids);
            free (value[0]);
            free (value);
            return GS_FAILED;
        }

        assert (value);
        assert (*value);

        char *db_string = *value;
        db_string_entry[0] = db_string;

        // Perform binary search with the database string as needle in the input string list haystack.
        // Since fetching the database string is a slow operation (i.e., from disk),
        // the search is performed in the input list list (i.e., in memory). The worse case is that the
        // entire string database must be loaded one by one into main memory to compute the result.
        // The theoretical complexity of this search is O(m*log2(n)) where m is the number of reads in the
        // database file and n is the number of input strings

        if (bsearch(db_string_entry, strings, num_strings, sizeof(char **), comp_str) != NULL) {
            if (strings_found) {
                db_strings_match_ids[result_size] = *id_cursor;
            }
            if (strings_not_found) {
                strings_found_result[result_size] = *db_string_entry;

            }
            if (++result_size == num_strings) {
                // Found all string
                break;
            }
        }
    }

    if (strings_found) {
        *strings_found = db_strings_match_ids;
    }

    if (strings_not_found) {
        size_t num_not_found = num_strings - result_size;
        char **needle = GS_REQUIRE_MALLOC(sizeof(char *));
        strings_not_found_result = GS_REQUIRE_MALLOC(num_not_found * sizeof(char *));
        int j = 0;
        for (int i = 0; i < num_strings; i++) {
            needle[0] = strings[i];
            if (bsearch(needle, strings_found_result, result_size, sizeof(char **), comp_str) == NULL) {
                strings_not_found_result[j++] = *needle;
            }
        }
        free (needle);
        *strings_not_found = strings_not_found_result;
    }

    free(db_string_ids);
    free(db_string_entry);
    *num_strings_found = result_size;
    return GS_SUCCESS;
}

string_id_t *database_string_fullscan(size_t *num_strings, gs_status_t *status, database_t *db)
{
    string_lookup_header_t  lookup_header;
    string_id_t            *result;
    size_t                  result_size = 0;

    database_lock(db);

    if (fseek(db->string_lookup, 0, SEEK_SET) != 0) {
        // Seeking to header position failed. Cancel operation.
        GS_OPTIONAL(status != NULL, *status = GS_FAILED);
        database_unlock(db);
        return NULL;
    }

    if (fread(&lookup_header, sizeof(string_lookup_header_t), 1, db->string_lookup) != 1) {
        // Lookup header read failed. Thus, unable to get maximum number of stored strings. Cancel operation.
        GS_OPTIONAL(status != NULL, *status = GS_FAILED);
        database_unlock(db);
        return NULL;
    }

    result = GS_REQUIRE_MALLOC(lookup_header.id_cursor * sizeof(string_id_t));

    for (size_t i = 0; i < lookup_header.id_cursor; i++) {
        string_lookup_entry_t entry;
        if (fread(&entry, sizeof(string_lookup_entry_t), 1, db->string_lookup) != 1) {
            // Read of one index entry failed for some reasons. Cancel operation and free resources
            free (result);
            GS_OPTIONAL(status != NULL, *status = GS_FAILED);
            database_unlock(db);
            return NULL;
        }
        if (entry.flags.in_use) {
            result[result_size++] = entry.string_offset;
        }
    }

    GS_OPTIONAL(status != NULL, *status = GS_SUCCESS);
    *num_strings = result_size;
    database_unlock(db);
    return result;
}

#define FREE_STR_VEC(vec, size)         \
{                                       \
    for (size_t j = 0; j < size; j++) { \
        free (vec[j]);                  \
    }                                   \
    free (vec);                         \
}

#define EXIT_STRING_READ_FN(db, vec, size, status, status_val)  \
{                                                               \
    FREE_STR_VEC(vec, size);                                    \
    GS_OPTIONAL(status != NULL, *status = status_val);          \
    return NULL;                                                \
}

char **database_string_read(gs_status_t *status, database_t *db, const string_id_t *string_ids, size_t num_strings)
{
    size_t                    max_offset;

    if (num_strings == 0) {
        GS_OPTIONAL(status != NULL, *status = GS_ILLEGALARG);
        return NULL;
    }

    char **result = GS_REQUIRE_MALLOC(num_strings * sizeof(char *));

    if (fseek(db->string_records, 0L, SEEK_END) != 0) {
        // Seeking error, unable to receive the size of the file
        GS_OPTIONAL(status != NULL, *status = GS_READ_FAILED);
        free (result);
        return NULL;
    };

    max_offset = ftell(db->string_records);

    if (fseek(db->string_records, sizeof(string_records_header_t), SEEK_SET) != 0) {
        // Seeking error, unable to load current header contents
        GS_OPTIONAL(status != NULL, *status = GS_READ_FAILED);
        free (result);
        return NULL;
    };

    for (size_t i = 0; i < num_strings; i++) {
        string_id_t string_id = string_ids[i];

        // In case the current id (i.e., the offset) is illegal (i.e., larger than maximum offset),
        // free up all resources for the result so far and exit that function
        if (string_id + 1 >= max_offset) {
            EXIT_STRING_READ_FN(db, result, i, status, GS_FAILED);
        }

        // Otherwise, read string without touching the index
        if (fseek(db->string_records, string_id, SEEK_SET) != 0) {
            EXIT_STRING_READ_FN(db, result, i, status, GS_FAILED);
        }

        string_record_entry_header_t entry_header;
        if (fread(&entry_header, sizeof(string_record_entry_header_t), 1, db->string_records) != 1) {
            EXIT_STRING_READ_FN(db, result, i, status, GS_FAILED);
        }

        result[i] = GS_REQUIRE_MALLOC(entry_header.string_length + 1);
        result[i][entry_header.string_length] = '\0';


        if (fread(result[i], entry_header.string_length, 1, db->string_records) != 1) {
            EXIT_STRING_READ_FN(db, result, i, status, GS_FAILED);
        }
    }

    GS_OPTIONAL(status != NULL, *status = GS_SUCCESS);
    return result;
}

prop_id_t *database_create_prop(database_t *db, const target_t *target, const char *key, const value_t *value,
                                const timespan_t *lifetime)
{
    return GS_FAILED;
}

gs_status_t database_close(database_t *db)
{
    fclose(db->node_records);
    fclose(db->node_heads);
    fclose(db->edge_records);
    fclose(db->string_lookup);
    fclose(db->string_records);
    apr_pool_destroy(db->apr_pool);
    free (db);
    return GS_SUCCESS;
}

static inline gs_status_t exists_db(database_t *database)
{
    return (files_exists(database->edges_db_path) && files_exists(database->nodes_records_path) &&
            files_exists(database->nodes_heads_path) && files_exists(database->string_lookup_path) &&
            files_exists(database->string_records_path)) ?
           GS_TRUE : GS_FALSE;
}

static inline gs_status_t unsafe_nodes_db_autoresize(database_t *db, node_records_header_t *header, size_t num_nodes)
{
    if (header->next_node_slot_id + num_nodes > header->capacity) {
        // The capacity_in_byte of the records file is too less. Thus, expand file.

        size_t expand_size = 1.7f * (1 + num_nodes);
        fseek(db->node_records, 0, SEEK_END);

        database_node_t empty = {
                .flags.in_use = FALSE
        };

        for (int i = 0; i < expand_size; i++) {
            if (fwrite(&empty, sizeof (database_node_t), 1, db->node_records) != 1) {
                return GS_FAILED;
            }
        }
        header->capacity += expand_size;
    }
    return GS_SUCCESS;
}

static inline gs_status_t unsafe_read_node_records_header(node_records_header_t *header, database_t *database)
{
    fseek(database->node_records, 0, SEEK_SET);
    return (fread(header, sizeof(node_records_header_t), 1, database->node_records) == 1 ? GS_SUCCESS : GS_FAILED);
}

static inline gs_status_t unsafe_read_node_index_header(node_index_header_t *header, database_t *database)
{
    fseek(database->node_heads, 0, SEEK_SET);
    return (fread(header, sizeof(node_index_header_t), 1, database->node_heads) == 1 ? GS_SUCCESS : GS_FAILED);
}

static inline gs_status_t unsafe_update_node_records_header(database_t *database, const node_records_header_t *header)
{
    fseek(database->node_records, 0, SEEK_SET);
    gs_status_t result = fwrite(header, sizeof(node_records_header_t), 1, database->node_records) == 1 ? GS_SUCCESS : GS_FAILED;
    fflush(database->node_records);
    return result;
}

static inline gs_status_t unsafe_update_node_index_header(database_t *database, const node_index_header_t *header)
{
    fseek(database->node_heads, 0, SEEK_SET);
    gs_status_t result = fwrite(header, sizeof(node_index_header_t), 1, database->node_heads) == 1 ? GS_SUCCESS : GS_FAILED;
    fflush(database->node_heads);
    return result;
}

static inline node_slot_id_t unsafe_read_node(database_node_t *node, node_id_t node_id, database_t *db)
{
    node_index_entry_t node_1st_version_slot;
    // Seek to next index slot in the node index file
    NODE_INDEX_FILE_SEEK(db, node_id);
    fread(&node_1st_version_slot, sizeof(node_index_entry_t), 1, db->node_heads);

    NODE_RECORDS_FILE_SEEK(db, node_1st_version_slot);
    fread(node, sizeof(database_node_t), 1, db->node_records);

    return node_1st_version_slot;
}

static inline node_slot_id_t unsafe_node_last_version(database_node_t *last, const database_node_t *start,
                                                      node_slot_id_t start_slot, database_t *db)
{
    database_node_t cursor = *start;
    offset_t last_node_slot = start_slot;
    while (cursor.flags.has_version_next) {
        NODE_RECORDS_FILE_SEEK(db, cursor.version_next);
        last_node_slot = cursor.version_next;
        fread(&cursor, sizeof(database_node_t), 1, db->node_records);
    }
    *last = cursor;
    return last_node_slot;
}

static inline gs_status_t unsafe_node_new_version(node_slot_id_t *cpy_slot_id, database_node_t *cpy,
                                                  database_node_t *original, database_t *db,
                                                  size_t approx_num_new_nodes,
                                                  node_slot_id_t original_slot)
{
    node_records_header_t records_header;
    gs_status_t           status;
    unsigned              max_retry;
    database_node_t       local_cpy, local_original;

    unsafe_read_node_records_header(&records_header, db);

    if (unsafe_nodes_db_autoresize(db, &records_header, approx_num_new_nodes) != GS_SUCCESS) {
        return GS_FAILED;
    }

    // Seek to next free slot in node records file
    NODE_RECORDS_FILE_SEEK(db, records_header.next_node_slot_id);
    *cpy_slot_id = records_header.next_node_slot_id;

    assert (original->flags.has_version_next == FALSE);

    // Create shallow local_cpy of node, update the modification timestamp, and link to the old version
    // Make local copies of "cpy" and "original" argument to corruption on these objects in case
    // changes are not written to the file
    local_cpy = *original;
    local_cpy.creation_time = (timestamp_t) time(NULL);
    local_cpy.flags.has_version_prev = TRUE;
    local_cpy.version_prev = original_slot;

    // Modify version pointer of the old version to point to the new version
    local_original = *original;
    local_original.flags.has_version_next = TRUE;
    local_original.version_next = *cpy_slot_id;

    // Write new version of node to the node storage
    if (fwrite(&local_cpy, sizeof(database_node_t), 1, db->node_records) != 1) {
        // In case this write fails, stop here and return to the caller.
        // Since there are no changes to original, this case is just a reject of the request
        // to create a new node.
        return GS_FAILED;
    }

    // Modify the old version to point to the new version
    NODE_RECORDS_FILE_SEEK(db, original_slot);
    if (fwrite(&local_original, sizeof(database_node_t), 1, db->node_records) != 1) {
        // In this case, the new version is physically written to the database file but neither is that
        // change reflected in the node records file header nor in the original version. Hence, aborting
        // here is just a reject of the request to create a new node.
        return GS_FAILED;
    }



    // TODO: cpy all properties and all edges!

    // Prepare registration of the new node in the header
    records_header.next_node_slot_id++;

    // Update records header in order to store that new node records were written. Give this 100 tries or fails.
    max_retry = 100;
    do {
        status = unsafe_update_node_records_header(db, &records_header);
    } while (status != GS_SUCCESS && max_retry--);

    if (status != GS_SUCCESS) {
        // The header is not updated. Whenever another node is now added, 'original' will point to this new
        // node since its 'next' pointer is already updated. To solve this, try a rollback on the 'next' pointer
        // change

        // Write the old version of node to the node storage
        NODE_RECORDS_FILE_SEEK(db, original_slot);
        if (fwrite(original, sizeof(database_node_t), 1, db->node_records) != 1) {
            // Rollback failed
            // TODO: Handle the rollback-failed for "new version of copy"
            panic("Node store corruption: node '%lld' will point to undefined successor version (file '%s')",
                   original->unique_id, db->nodes_records_path);
            return GS_FAILED;
        } else {
            // Rollback was successful
            fflush(db->node_records);
            return GS_FAILED;
        }

    }

    // Changes are written to the database file. Thus, update the input parameters to reflect that change
    *cpy = local_cpy;
    *original = local_original;

    return GS_SUCCESS;
}

static inline gs_status_t unsafe_node_adjust_lifetime(database_node_t *node, node_slot_id_t slot_id,
                                                      database_t *db, const timespan_t *lifetime)
{
    node->lifetime = *lifetime;
    NODE_RECORDS_FILE_SEEK(db, slot_id);
    if (fwrite(node, sizeof(database_node_t), 1, db->node_records) != 1) {
        // Lifetime of node cannot be updated, reject request
        return GS_FAILED;
    } else {
        return GS_SUCCESS;
    }
}

static inline gs_status_t unsafe_read_string_records_header(string_records_header_t *header, database_t *db)
{
    fseek(db->string_records, 0, SEEK_SET);
    return (fread(header, sizeof(string_records_header_t), 1, db->string_records) == 1 ? GS_SUCCESS : GS_FAILED);
}

static inline gs_status_t unsafe_read_string_lookup_header(string_lookup_header_t *header, database_t *db)
{
    fseek(db->string_lookup, 0, SEEK_SET);
    return (fread(header, sizeof(string_lookup_header_t), 1, db->string_lookup) == 1 ? GS_SUCCESS : GS_FAILED);
}

static inline gs_status_t unsafe_update_string_records_header(database_t *database,
                                                              const string_records_header_t *header)
{
    fseek(database->string_records, 0, SEEK_SET);
    gs_status_t result = (fwrite(header, sizeof(string_records_header_t), 1, database->string_records) == 1 ?
                          GS_SUCCESS : GS_FAILED);
    fflush(database->string_records);
    return result;
}

static inline gs_status_t unsafe_update_string_index_header(database_t *database,
                                                            const string_lookup_header_t *header)
{
    fseek(database->string_lookup, 0, SEEK_SET);
    gs_status_t result = (fwrite(header, sizeof(string_lookup_header_t), 1, database->string_lookup) == 1 ?
                          GS_SUCCESS : GS_FAILED);
    fflush(database->string_lookup);
    return result;
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
    node_records_header_t records_header = {
        .next_node_slot_id = 0,
        .next_node_id      = 0,
        .capacity     = DB_NODE_DEFAULT_CAPACITY
    };
    database_node_t node = {
        .flags.in_use = FALSE
    };

    node_index_header_t index_header = {
        .node_cursor      = 0
    };

    // Initialize node record file
    if ((database->node_records = fopen(database->nodes_records_path, "wb")) == NULL)
        return GS_FAILED;

    if (fwrite(&records_header, sizeof (node_records_header_t), 1, database->node_records) != 1)
        return GS_FAILED;

    for (int i = 0; i < records_header.capacity; i++) {
        if (fwrite(&node, sizeof (database_node_t), 1, database->node_records) != 1)
            return GS_FAILED;
    }

    // Initial node index file
    if ((database->node_heads = fopen(database->nodes_heads_path, "wb")) == NULL)
        return GS_FAILED;
    if (fwrite(&index_header, sizeof (node_index_header_t), 1, database->node_heads) != 1)
        return GS_FAILED;

    fflush(database->node_records);
    fflush(database->node_heads);

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

    if ((database->edge_records = fopen(database->edges_db_path, "wb")) == NULL)
        return GS_FAILED;

    if (fwrite(&header, sizeof (edges_header_t), 1, database->edge_records) != 1)
        return GS_FAILED;

    for (int i = 0; i < header.capacity; i++) {
        if (fwrite(&edge, sizeof(edges_t), 1, database->edge_records) != 1)
            return GS_FAILED;
    }

    fflush(database->edge_records);

    return GS_SUCCESS;
}

static inline gs_status_t create_stringpool_db(database_t *database)
{
    char empty;

    string_records_header_t records_header = {
        .size_in_byte           = 0,
        .capacity_in_byte       = DB_STRING_POOL_CAPACITY_BYTE,
        .write_offset           = sizeof(string_records_header_t)
    };

    if ((database->string_records = fopen(database->string_records_path, "wb")) == NULL)
        return GS_FAILED;

    if ((database->string_lookup = fopen(database->string_lookup_path, "wb")) == NULL)
        return GS_FAILED;

    if (fwrite(&records_header, sizeof (string_records_header_t), 1, database->string_records) != 1)
        return GS_FAILED;

    fseek(database->string_records, sizeof(string_records_header_t) + records_header.capacity_in_byte, SEEK_SET);
    if (fwrite(&empty, sizeof(char), 1, database->string_records) != 1) {
            return GS_FAILED;
    }

    string_lookup_header_t lookup_header = {
        .id_cursor   = 0,
        .capacity    = DB_STRING_POOL_LOOKUP_SLOT_CAPACITY
    };

    string_lookup_entry_t entry = {
        .flags.in_use = FALSE
    };

    if (fwrite(&lookup_header, sizeof(string_lookup_header_t), 1, database->string_lookup) != 1) {
        return GS_FAILED;
    }

    for (int i = 0; i < lookup_header.capacity; i++) {
        if (fwrite(&entry, sizeof(string_lookup_entry_t), 1, database->string_lookup) != 1)
            return GS_FAILED;
    }

    fflush(database->string_records);
    fflush(database->string_lookup);

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
    if ((database->node_records = fopen(database->nodes_records_path, "r+b")) == NULL)
        return GS_FAILED;
    if ((database->node_heads = fopen(database->nodes_heads_path, "r+b")) == NULL)
        return GS_FAILED;
    return GS_SUCCESS;
}

static inline gs_status_t open_edges_db(database_t *database)
{
    if ((database->edge_records = fopen(database->edges_db_path, "r+b")) == NULL)
        return GS_FAILED;
    return GS_SUCCESS;
}

static inline gs_status_t open_stringpool_db(database_t *database)
{
    if ((database->string_lookup = fopen(database->string_lookup_path, "r+b")) == NULL)
        return GS_FAILED;
    if ((database->string_records = fopen(database->string_records_path, "r+b")) == NULL)
        return GS_FAILED;
    return GS_SUCCESS;
}