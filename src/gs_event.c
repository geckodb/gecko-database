#include <gs_event.h>

typedef struct gs_event_t {
    gs_signal_type_e signal;
    gs_event_tag_e tag;
    void *sender;
    void *payload;

    void (*dispose)(gs_event_t *self);
} gs_event_t;

gs_event_t *gs_event_new(gs_signal_type_e s, gs_event_tag_e t, void *sdr, void *p, gs_event_dispose d)
{
    gs_event_t *event = GS_REQUIRE_MALLOC(sizeof(gs_event_t));
    event->signal = s;
    event->tag = t;
    event->sender = sdr;
    event->payload = p;
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

void *gs_event_get_payload(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    return (event->payload);
}

gs_event_t *gs_event_dispatcher_shutdown(gs_dispatcher_t *dispatcher)
{
    return gs_event_new(GS_SIG_SHUTDOWN, GS_EVENT_TAG_DISPATCHER, NULL, dispatcher, NULL);
}

gs_event_t *gs_event_heartbeat_new(gs_dispatcher_t *dispatcher)
{
    return gs_event_new(GS_SIG_HEARTBEAT, GS_EVENT_TAG_DISPATCHER, NULL, dispatcher, NULL);
}

//----------------------------------------------------------------------------------------------------------------------

gs_event_t *gs_event_gridstore_test(gs_gridstore_t *gridstore)
{
    return gs_event_new(GS_SIG_TEST, GS_EVENT_TAG_GRIDSTORE, NULL, gridstore, NULL);
}