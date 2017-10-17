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

void tuplet_open(tuplet_t *dst, struct frag_t *frag, tuplet_id_t tuplet_id)
{
    if (frag->ntuplets > 0) {
        GS_REQUIRE_NONNULL(frag);
        GS_REQUIRE_NONNULL(frag->_open);
        frag->_open(dst, frag, tuplet_id);
    }
}

bool tuplet_next(tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->_next);
    return tuplet->_next(tuplet);
}

void tuplet_rewind(tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    frag_t *frag = tuplet->fragment;
    tuplet_open(tuplet, frag, 0);
}

void tuplet_set_null(tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->_set_null);
    tuplet->_set_null(tuplet);
}

bool tuplet_is_null(tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->_is_null);
    return (tuplet->_is_null(tuplet));
}

size_t tuplet_size(tuplet_t *tuplet)
{
    GS_REQUIRE_NONNULL(tuplet);
    GS_REQUIRE_NONNULL(tuplet->fragment);
    return tuplet->fragment->tuplet_size;
}

void *tuplet_update(void *dst, schema_t *frag, attr_id_t attr_id, void *src)
{
    panic(NOTIMPLEMENTED, to_string(tuplet_update))
    return NULL;
}

size_t tuplet_size_by_schema(const schema_t *schema)
{
    size_t total = 0;
    for (size_t attr_idx = 0; attr_idx < schema->attr->num_elements; attr_idx++) {
        const struct attr_t *attr = schema_attr_by_id(schema, attr_idx);
        total += attr_total_size(attr);
    }
    return total;
}

enum field_type tuplet_field_type(const tuplet_t *tuplet, attr_id_t id)
{
    assert(tuplet);
    return frag_field_type(tuplet->fragment, id);
}

const char *tuplet_format_str(enum tuplet_format format)
{
    switch (format) {
        case TF_NSM: return "row";
        case TF_DSM: return "column";
        default: perror("Unknown tuple format"); abort();
    }
}
