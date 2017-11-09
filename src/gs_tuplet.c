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

#include <gs_tuplet.h>
#include <gs_tuplet_field.h>
#include <gs_schema.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_tuplet_open(gs_tuplet_t *dst, struct gs_frag_t *frag, gs_tuplet_id_t tuplet_id)
{
    if (frag->ntuplets > 0) {
        GS_REQUIRE_NONNULL(frag);
        GS_REQUIRE_NONNULL(frag->_open);
        frag->_open(dst, frag, tuplet_id);
    }
}

bool gs_tuplet_next(gs_tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->_next);
    return tuplet->_next(tuplet);
}

void gs_tuplet_rewind(gs_tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    gs_frag_t *frag = tuplet->fragment;
    gs_tuplet_open(tuplet, frag, 0);
}

void gs_tuplet_set_null(gs_tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->_set_null);
    tuplet->_set_null(tuplet);
}

bool gs_tuplet_is_null(gs_tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->_is_null);
    return (tuplet->_is_null(tuplet));
}

size_t gs_tuplet_size(gs_tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->fragment);
    return tuplet->fragment->tuplet_size;
}

void *gs_tuplet_update(void *dst, gs_schema_t *frag, gs_attr_id_t attr_id, void *src)
{
    panic(NOTIMPLEMENTED, to_string(gs_tuplet_update))
    return NULL;
}

size_t gs_tuplet_size_by_schema(const gs_schema_t *schema)
{
    size_t total = 0;
    for (size_t attr_idx = 0; attr_idx < schema->attr->num_elements; attr_idx++) {
        const struct gs_attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
        total += gs_attr_total_size(attr);
    }
    return total;
}

enum gs_field_type_e gs_tuplet_field_type(const gs_tuplet_t *tuplet, gs_attr_id_t id)
{
    assert(tuplet);
    return gs_frag_field_type(tuplet->fragment, id);
}

const char *gs_tuplet_format_str(enum gs_tuplet_format_e format)
{
    switch (format) {
        case TF_NSM: return "row";
        case TF_DSM: return "column";
        default: perror("Unknown tuple format"); abort();
    }
}
