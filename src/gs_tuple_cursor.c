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
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_tuple_cursor.h>

void gs_tuple_cursor_create(gs_tuple_cursor_t *cursor, struct gs_table_t *context, gs_tuple_id_t *tuple_ids,
                            size_t ntuple_ids)
{
    GS_REQUIRE_NONNULL(cursor);
    cursor->context = context;
    cursor->ntuple_ids = ntuple_ids;
    cursor->tuple_ids = tuple_ids;
    gs_tuple_cursor_rewind(cursor);
}

void gs_tuple_cursor_dispose(gs_tuple_cursor_t *cursor)
{
    GS_REQUIRE_NONNULL(cursor);
    free (cursor->tuple_ids);
}

void gs_tuple_cursor_rewind(gs_tuple_cursor_t *cursor)
{
    GS_REQUIRE_NONNULL(cursor);
    cursor->tuple_id_cursor = 0;
}

bool gs_tuple_cursor_next(gs_tuple_t *tuple, gs_tuple_cursor_t *cursor)
{
    GS_REQUIRE_NONNULL(cursor);
    GS_REQUIRE_NONNULL(tuple);
    if (cursor->tuple_id_cursor < cursor->ntuple_ids) {
        gs_tuple_open(tuple, cursor->context, cursor->tuple_ids[cursor->tuple_id_cursor++]);
        return true;
    } else return false;
}
