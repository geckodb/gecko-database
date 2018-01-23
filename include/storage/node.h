#ifndef GECKO_GS_NODE_H
#define GECKO_GS_NODE_H

/* forwarding */
typedef struct pool_t;
typedef struct node_cursor_t;
typedef struct property_t;
typedef struct mempool;

typedef struct node_t;

gs_status_t node_create(node_cursor_t *cursor, pool_t *pool, size_t num_nodes, timespan_t timespan, mempool *mempool);

gs_status_t node_add_prop(node_cursor_t *cursor, const property_t *property_t);

gs_status_t node_delete(node_cursor_t *cursor);

#endif
