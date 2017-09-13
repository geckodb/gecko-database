// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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

#include <tuple.h>
#include <grid.h>

void gs_tuple_open(tuple_t *tuple, const struct grid_table_t *table, tuple_id_t tuple_id)
{
    REQUIRE_NONNULL(tuple);
    REQUIRE_NONNULL(table);
    tuple->table = table;
    tuple->tuple_id = tuple_id;
}

void gs_tuple_id_init(void *data)
{
    *((tuple_id_t *) data) = 0;
}

void gs_tuple_id_inc(void *data)
{
    *((tuple_id_t *) data) += 1;
}
