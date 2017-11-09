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

#include <gs_tuple.h>
#include <gs_grid.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------


void gs_tuple_open(gs_tuple_t *tuple, const struct gs_table_t *table, gs_tuple_id_t tuple_id)
{
    GS_REQUIRE_NONNULL(tuple);
    GS_REQUIRE_NONNULL(table);
    tuple->table = table;
    tuple->tuple_id = tuple_id;
}

void gs_tuple_id_init(void *data)
{
    *((gs_tuple_id_t *) data) = 0;
}

void gs_tuple_id_inc(void *data)
{
    *((gs_tuple_id_t *) data) += 1;
}
