#include <gs_event.h>
#include <gs_spinlock.h>

typedef struct gs_event_t {
    gs_signal_type_e        signal;
    gs_object_type_tag_e    sender_tag;
    gs_object_type_tag_e    receiver_tag;
    bool                    is_blocking;
    void                   *sender;
    void                   *receiver;
    void                   *data;
    void (*dispose)(gs_event_t *self);
} gs_event_t;

gs_event_t *gs_event_new(gs_signal_type_e s, gs_object_type_tag_e sdr_t, void *sdr, gs_object_type_tag_e rcvr_t,
                         void *rcvr, void *data, gs_event_dispose d)
{
    gs_event_t *event = GS_REQUIRE_MALLOC(sizeof(gs_event_t));
    event->signal = s;
    event->sender_tag = sdr_t;
    event->sender = sdr;
    event->receiver_tag = rcvr_t;
    event->receiver = rcvr;
    event->data = data;
    event->dispose = d;
    event->is_blocking = false;
    return event;
}

void gs_event_free(gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    if (event->dispose != NULL) {
        event->dispose(event);
    }
    free(event);
}

gs_signal_type_e gs_event_get_signal(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    return (event->signal);
}

void *gs_event_get_data(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    return (event->data);
}

 void *blocking_event_wrap_data(volatile gs_spinlock_t *lock, gs_event_t *contained_event);
 void blocking_event_unlock_and_dispose(gs_event_t *blocking_event);

gs_status_t gs_event_get_subject(gs_object_type_tag_e *type_tag, void **ptr, const gs_event_t *event, gs_subject_kind_e subj)
{
    if (event != NULL) {
        if (type_tag != NULL) {
            *type_tag = (subj == GS_SENDER ? event->sender_tag : event->receiver_tag);
        }
        if (ptr != NULL) {
            *ptr = (subj == GS_SENDER ? event->sender : event->receiver);
        }
        return GS_SUCCESS;
    } else return GS_ILLEGALARG;
}

gs_event_t *gs_event_dispatcher_shutdown(gs_dispatcher_t *dispatcher)
{
    return gs_event_new(GS_SIG_SHUTDOWN, GS_OBJECT_TYPE_NONE, NULL, GS_OBJECT_TYPE_DISPATCHER, dispatcher, NULL, NULL);
}

gs_event_t *gs_event_shell_shutdown(gs_dispatcher_t *dispatcher, gs_shell_t *shell)
{
    return gs_event_new(GS_SIG_SHUTDOWN, GS_OBJECT_TYPE_NONE, NULL, GS_OBJECT_TYPE_SHELL, shell, NULL, NULL);
}

gs_event_t *gs_event_server_shutdown(gs_dispatcher_t *dispatcher, gs_server_t *server)
{
    return gs_event_new(GS_SIG_SHUTDOWN, GS_OBJECT_TYPE_NONE, NULL, GS_OBJECT_TYPE_SERVER, server, NULL, NULL);
}

gs_event_t *gs_event_gridstore_shutdown(gs_dispatcher_t *dispatcher, gs_gridstore_t *gridstore)
{
    return gs_event_new(GS_SIG_SHUTDOWN, GS_OBJECT_TYPE_NONE, NULL, GS_OBJECT_TYPE_GRIDSTORE, gridstore, NULL, NULL);
}

gs_event_t *gs_event_system_exit(gs_dispatcher_t *dispatcher, gs_object_type_tag_e sender_tag, void *sender)
{
    GS_REQUIRE_NONNULL(sender);
    return gs_event_new(GS_SIG_SYSEXIT, sender_tag, sender, GS_OBJECT_TYPE_SYSTEM, NULL, NULL, NULL);
}

gs_event_t *gs_event_heartbeat_new(gs_dispatcher_t *dispatcher)
{
    return gs_event_new(GS_SIG_HEARTBEAT, GS_OBJECT_TYPE_NONE, NULL, GS_OBJECT_TYPE_DISPATCHER, dispatcher, NULL, NULL);
}

//----------------------------------------------------------------------------------------------------------------------

gs_event_t *gs_event_gridstore_test(gs_gridstore_t *gridstore)
{
    return gs_event_new(GS_SIG_TEST, GS_OBJECT_TYPE_NONE, NULL, GS_OBJECT_TYPE_GRIDSTORE, gridstore, NULL, NULL);
}

gs_event_t *gs_event_gridstore_invoke()
{
    return gs_event_new(GS_SIG_INVOKE, GS_OBJECT_TYPE_NONE, NULL, GS_OBJECT_TYPE_GRIDSTORE, NULL, NULL, NULL);
}


typedef struct blocking_event_args_t
{
    volatile gs_spinlock_t *lock;
    gs_event_t *contained_event;
} blocking_event_args_t;

gs_event_t *gs_event_new_blocking(volatile gs_spinlock_t *lock, gs_event_t *contained_event)
{
    gs_event_t *result = gs_event_new(contained_event->signal,
                                      contained_event->sender_tag,
                                      contained_event->sender,
                                      contained_event->receiver_tag,
                                      contained_event->receiver,
                                      blocking_event_wrap_data(lock, contained_event),
                                      NULL);
    result->is_blocking = true;
    result->dispose = blocking_event_unlock_and_dispose;
    return result;
}

 void *blocking_event_wrap_data(volatile gs_spinlock_t *lock, gs_event_t *contained_event)
{
    blocking_event_args_t *result = GS_REQUIRE_MALLOC(sizeof(blocking_event_args_t));
    result->lock = lock;
    result->contained_event = contained_event;
    return result;
}

 void blocking_event_unlock_and_dispose(gs_event_t *blocking_event)
{
    assert (blocking_event->is_blocking);
    blocking_event_args_t *args = (blocking_event_args_t *) blocking_event->data;
    gs_spinlock_unlock(args->lock);
    gs_event_free(args->contained_event);
    free (blocking_event->data);
}
