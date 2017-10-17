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

#include <gs.h>
#include <gs_attr.h>
#include <gs_frag.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct schema_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct tuplet_t {
    struct frag_t *fragment; /*<! fragment in which this tuplet exists */
    tuplet_id_t tuplet_id; /*<! number of this tuplet inside the fragment */
    void *attr_base; /*<! pointer in fragment where first attribute of this tuplet is located */

    /* operations */
    bool (*_next)(struct tuplet_t *self); /* seeks to the next tuplet inside this fragment */
    void (*_open)(struct tuplet_field_t *dst, struct tuplet_t *self); /*<! access the attr_value_ptr data of this tuplet */
    void (*_update)(struct tuplet_t *self, const void *data); /*<! updates all fields of this tuplet and moves to next */
    void (*_set_null)(struct tuplet_t *self); /*<! updates all fields of this tuplet to NULL, and moves to next */
    void (*_delete)(struct tuplet_t *self); /* request to delete this tuplet from fragment */
    bool (*_is_null)(struct tuplet_t *self); /*<! checks whether this tuplet is NULL entirely */

} tuplet_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

/*!
 * @brief Opens the the tuplet associated with a given tuplet id that is located in the fragment <i>frag</i>.
 *
 * To navigate from one tuplet to another, the function gs_tuplet_next() should be used.
 *
 * The returned tuplet is allocated on the heap. To release these resources, the tuplet must be either closed
 * explicitly by calling gs_tuplet_close() or it gets automatically released when gs_tuplet_next()
 * reaches the end of the fragment.
 *
 *
 * @param [in] frag The fragment. Must be non-null.
 *        [in] tuplet_id the tuplet id. Must be valid.
 * @return A pointer to the first tuplet in <i>frag</i>, or <b>NULL</b> if the fragment does not contains any tuplets.
 * */
void tuplet_open(tuplet_t *dst, struct frag_t *frag, tuplet_id_t tuplet_id);

/*!
 * @brief Moves the input tuplet cursor to its successor inside its fragment.
 *
 * For navigation from one tuplet to another this function should be used. The order in which tuplets are enumerated
 * is fragment-specific and it's not guaranteed that the tuplet identifier of the input tuplet is less than the tuplet
 * identifier of its successor. However, its guaranteed that all tuplets inside a fragment are returned if the first
 * tuplet was received by a call to gs_tuplet_open() and gs_tuplet_next() is called until gs_tuplet_next() returns
 * <b>false</b>.
 *
 * If the end of the fragment is reached, this functions returns <b>false</b> and the input tuplet is automatically
 * closed via a call to gs_tuplet_close().
 *
 * @param tuplet A valid tuplet inside a fragment (must be non-null)
 * @return <b>true</b> is the tuplet cursor was moved to its successor and the end of the fragment was not reached,
 * or <b>false</b> otherwise.
 */
bool tuplet_next(tuplet_t *tuplet);

/*!
 * @brief Resets the tuplet pointer to the begin of the first tuplet in its fragment.
 *
 * The input tuplet is closed automatically via a call to gs_tuplet_close(). Afterwards the first tuplet is identified
 * by calling gs_tuplet_open() and returned to the caller.
 *
 * @param tuplet A valid tuplet inside a fragment (mus be non-null)
 * @return The first tuplet in the fragment of the input tuplet.
 */
void tuplet_rewind(tuplet_t *tuplet);

void tuplet_set_null(tuplet_t *tuplet);

bool tuplet_is_null(tuplet_t *tuplet);

size_t tuplet_size(tuplet_t *tuplet);

void *tuplet_update(void *dst, schema_t *frag, attr_id_t attr_id, void *src);

size_t tuplet_size_by_schema(const schema_t *schema);

enum field_type tuplet_field_type(const tuplet_t *tuplet, attr_id_t id);

size_t tuplet_printlen(const attr_t *attr, const void *field_data);

const char *tuplet_format_str(enum tuplet_format format);
