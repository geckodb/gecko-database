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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gs_tuple.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct gs_tuple_cursor_t {
    struct gs_table_t *context;
    gs_tuple_id_t *tuple_ids;
    size_t ntuple_ids;
    size_t tuple_id_cursor;
} gs_tuple_cursor_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_tuple_cursor_create(gs_tuple_cursor_t *cursor, struct gs_table_t *context, gs_tuple_id_t *tuple_ids,
                            size_t ntuple_ids);
void gs_tuple_cursor_dispose(gs_tuple_cursor_t *cursor);
void gs_tuple_cursor_rewind(gs_tuple_cursor_t *cursor);
bool gs_tuple_cursor_next(gs_tuple_t *tuple, gs_tuple_cursor_t *cursor);
