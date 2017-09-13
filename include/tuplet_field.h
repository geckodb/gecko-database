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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <tuplet.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct tuplet_field_t {
    tuplet_t *tuplet; /*<! pointer to tuplet to which this attr_value_ptr belongs to */

    attr_id_t attr_id; /*<! current attribute tuplet_id to which this tuplet is seeked */
    void *attr_value_ptr; /*<! pointer in data in 'tuplet_base' of attr_value_ptr content of attribute 'attr_id */

    /* operations */
    bool (*_next)(struct tuplet_field_t *self, bool auto_next); /* seeks to the next attr_value_ptr inside this tuplet */
    bool (*_seek)(struct tuplet_field_t *self, attr_id_t attr_id); /* seeks the attr_value_ptr to this attribute */
    const void *(*_read)(struct tuplet_field_t *self); /* access current attr_value_ptr 'attr_id' of this tuplet */
    void (*_update)(struct tuplet_field_t *self, const void *data); /* update data of current attr_value_ptr 'attr_id' */
    void (*_set_null)(struct tuplet_field_t *self); /* set the attr_value_ptr for 'attr_id' to NULL */
    bool (*_is_null)(struct tuplet_field_t *self); /* returns true iff attr_value_ptr of 'attr_id' is null */
} tuplet_field_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_tuplet_field_open(tuplet_field_t *dst, tuplet_t *tuplet);

void gs_tuplet_field_seek(tuplet_field_t *dst, tuplet_t *tuplet, attr_id_t attr_id);

bool gs_tuplet_field_next(tuplet_field_t *field, bool auto_next);

const void *gs_tuplet_field_read(tuplet_field_t *field);

void gs_tuplet_field_update(tuplet_field_t *field, const void *data);

bool gs_tuplet_field_write(tuplet_field_t *field, const void *data, bool auto_next);

bool gs_tuplet_field_write_eval(tuplet_field_t *field, bool eval, bool auto_next);

void gs_tuplet_field_set_null(tuplet_field_t *field);

bool gs_tuplet_field_is_null(tuplet_field_t *field);

size_t gs_tuplet_field_size(tuplet_field_t *field);

size_t gs_attr_total_size(const struct attr_t *attr);

size_t gs_tuplet_field_get_printlen(const tuplet_field_t *field);

enum field_type gs_tuplet_field_get_type(const tuplet_field_t *field);

char *gs_tuplet_field_to_string(const tuplet_field_t *field);