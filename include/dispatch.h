#pragma once

#include <stdinc.h>

#include <apr.h>
#include <apr_pools.h>
#include <apr_queue.h>


#define MESSAGE_QUEUE_SIZE_MAX 100

typedef struct dispatcher_t
{
    bool is_running;
    apr_queue_t *queue;
    apr_pool_t *mem_pool;
    dict_t *handler_map;

} dispatcher_t;

typedef enum message_type {
    MESSAGE_SHUTDOWN
} message_type;

typedef struct event_t
{

} event_t;

typedef void (*message_handler_t)(const event_t *event);

typedef unsigned hander_id_t;


void dispatch_create(dispatcher_t *dispatcher);
void dispatch_dispose(dispatcher_t *dispatcher);
int dispatch_run(dispatcher_t *dispatcher);
hander_id_t dispatch_connect(dispatcher_t *dispatcher, message_type message, message_handler_t handler);
int dispatch_disconnect(dispatcher_t *dispatcher, hander_id_t handler_id);