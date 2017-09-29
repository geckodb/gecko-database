#include <gs_event.h>

typedef struct gs_event_t {
    gs_signal_type_e        signal;
    gs_object_type_tag_e    sender_tag;
    gs_object_type_tag_e    receiver_tag;
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