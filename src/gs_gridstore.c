// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see .

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_gridstore.h>
#include <gs_dispatcher.h>
#include <gs_collections.h>
#include <gs_database.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_gridstore_t {
    gs_vec_t *databases;
} gs_gridstore_t;

bool free_databases(void *capture, void *begin, void *end);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

GS_DECLARE(gs_status_t) gs_gridstore_create(gs_gridstore_t **gridstore)
{
    gs_gridstore_t *result = GS_REQUIRE_MALLOC(sizeof(gs_gridstore_t));
  //  result->databases = gs_vec_new(sizeof(gs_database_t *), 20);
    *gridstore = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_gridstore_dispose(gs_gridstore_t *gridstore)
{
    GS_REQUIRE_NONNULL(gridstore)
  //  gs_vec_free_ex(gridstore->databases, NULL, free_databases);
    free(gridstore);
    GS_DEBUG("gridstore %p was disposed", gridstore);
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_gridstore_handle_events(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_GRIDSTORE);

    //gs_gridstore_t   *self   = GS_EVENT_GET_RECEIVER(gs_gridstore_t);
    gs_signal_type_e  signal = GS_EVENT_GET_SIGNAL();

    switch (signal) {
        case GS_SIG_SHUTDOWN:
            printf("Grid store should shutdown, yeah\n");
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

bool free_databases(void *capture, void *begin, void *end)
{
    gs_database_t **it = begin;
    gs_database_t **tail = (gs_database_t **) end;
    while (it < tail) {
//        gs_database_dispose(*it);
        it++;
    }
    return true;
}