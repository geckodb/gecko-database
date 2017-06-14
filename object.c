// An implementation of base object for oop
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

#include <object.h>
#include <require.h>
#include <msg.h>

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void object_to_string(void *self, FILE *out);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void object_create(object_t *obj)
{
    require_nonnull(obj);
    *obj = (struct object_t) {
        .public = {
            .methods = {
                    .to_string_fn = object_to_string
            }
        }
    };
}

void object_override(object_t *obj, object_to_string_fn_t f)
{
    require_nonnull(obj);
    require_nonnull(f);
    obj->public.methods.to_string_fn = f;
}

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void object_to_string(void *self, FILE *out)
{
    require_nonnull(self);
    require_nonnull(out);
    fprintf(out, "object(adr=%p)", self);
}