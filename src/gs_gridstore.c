#include <gs_gridstore.h>
#include <gs_dispatcher.h>

typedef struct gs_gridstore_t {

} gs_gridstore_t;

GS_DECLARE(gs_status_t) gs_gridstore_create(gs_gridstore_t **gridstore)
{
    gs_gridstore_t *result = GS_REQUIRE_MALLOC(sizeof(gs_gridstore_t));
    *gridstore = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_gridstore_dispose(gs_gridstore_t **gridstore)
{
    GS_REQUIRE_NONNULL(gridstore)
    GS_REQUIRE_NONNULL(*gridstore)
    gs_gridstore_t *obj = *gridstore;
    free(obj);
    *gridstore = NULL;
    GS_DEBUG("gridstore %p was disposed", gridstore);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_gridstore_handle_events(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_GRIDSTORE);

    gs_gridstore_t   *self   = GS_EVENT_GET_RECEIVER(gs_gridstore_t);
    gs_signal_type_e  signal = GS_EVENT_GET_SIGNAL();

    switch (signal) {
        case GS_SIG_SHUTDOWN:
            printf("Gid store should shutdown, yeah\n");
            return GS_CATCHED;
        case GS_SIG_TEST:
            printf("Hey, yeah\n");
            return GS_CATCHED;
        case GS_SIG_INVOKE:
            GS_DEBUG2("Main Thread: invoke something");
            GS_DEBUG2("Main Thread: sleep...");



            GS_DEBUG2("Main Thread: wake up...");
            return GS_CATCHED;
        default:
        warn("gridstore %p received event for signal %d that is not handled", self, signal);
            return GS_SKIPPED;
    }
}