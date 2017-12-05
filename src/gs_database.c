#include <gs_database.h>

typedef struct gs_database_t
{
    char *name;
} gs_database_t;

GS_DECLARE(gs_status_t) gs_database_create(gs_database_t **database, const char *name)
{
    GS_REQUIRE_NONNULL(database);
    GS_REQUIRE_NONNULL(name);
    gs_database_t *result = GS_REQUIRE_MALLOC(sizeof(gs_database_t));
    result->name = strdup(name);
    *database = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_database_dispose(gs_database_t *database)
{
    GS_REQUIRE_NONNULL(database);
    free (database->name);
    free (database);
    return GS_SUCCESS;
}