#include <gs_gridstore.h>
#include <dispatcher.h>

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
    switch (gs_event_get_signal(event)) {
        case GS_SIG_TEST:
            printf("HEY!\n");
            return GS_CATCHED;
        default:
            warn("gridstore received event for signal %d that is not handled", gs_event_get_signal(event));
            return GS_SKIPPED;
    }
}