#pragma once

#include <stdinc.h>
#include <frag.h>

//#define DECLARE_TUPLET_INSERT(type_name, c_type, internal_type)                                                        \
//void *gs_insert_##type_name(void *dst, schema_t *schema, attr_id_t attr_id, const c_type *src);
//
//#define DECLARE_ARRAY_FIELD_INSERT(type_name, c_type, internal_type)                                                   \
//void *gs_insert_##type_name(void *dst, schema_t *schema, attr_id_t attr_id, const c_type *src);


struct schema_t;

typedef struct tuplet_t {
    struct fragment_t *fragment; /*<! fragment in which this tuplet exists */
    size_t id; /*<! number of this tuplet inside the fragment */
    void *attr_base; /*<! pointer in fragment where first attribute of this tuplet is located */

    /* operations */
    bool (*_next)(struct tuplet_t *self); /* seeks to the next tuplet inside this fragment */
    void (*_close)(struct tuplet_t *self); /* frees resources of this tuplet */
    struct field_t *(*_open)(struct tuplet_t *self); /*<! access the attr_value_ptr data of this tuplet */
    void (*_update)(struct tuplet_t *self, const void *data); /*<! updates all fields of this tuplet and moves to next */
    void (*_set_null)(struct tuplet_t *self); /*<! updates all fields of this tuplet to NULL, and moves to next */
    void (*_delete)(struct tuplet_t *self); /* request to delete this tuplet from fragment */
    bool (*_is_null)(struct tuplet_t *self); /*<! checks whether this tuplet is NULL entirely */

} tuplet_t;

/*!
 * @brief Opens the first tuplet that is located in the fragment <i>frag</i>.
 *
 * To navigate from one tuplet to another, the function gs_tuplet_next() should be used.
 *
 * The returned tuplet is allocated on the heap. To release these resources, the tuplet must be either closed
 * explicitly by calling gs_tuplet_close() or it gets automatically released when gs_tuplet_next()
 * reaches the end of the fragment.
 *
 * In case the
 *
 * @param [in] frag The fragment. Must be non-null.
 * @return A pointer to the first tuplet in <i>frag</i>, or <b>NULL</b> if the fragment does not contains any tuplets.
 * */
tuplet_t *gs_tuplet_open(struct fragment_t *frag);

/*!
 * @brief Closes a tuplet and frees up resources bound to this tuplet.
 *
 * Tuplets are allocated on the heap. When no further operations on tuplets are needed, they must be closed in order
 * to release resources (i.e., freeing space on the heap and fragment-specific resources). A closing operation is
 * either scheduled by a call to gs_tuplet_close() or when gs_tuplet_next() reaches the end of the fragment.
 * However, which resources are actually freed is fragment-type specific. It is not defined how a fragment behaves when
 * tuplets are not closed correctly.
 *
 * @param [in] tuplet The tuplet to be closed.
 */
void gs_tuplet_close(tuplet_t *tuplet);

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
bool gs_tuplet_next(tuplet_t *tuplet);

/*!
 * @brief Resets the tuplet pointer to the begin of the first tuplet in its fragment.
 *
 * The input tuplet is closed automatically via a call to gs_tuplet_close(). Afterwards the first tuplet is identified
 * by calling gs_tuplet_open() and returned to the caller.
 *
 * @param tuplet A valid tuplet inside a fragment (mus be non-null)
 * @return The first tuplet in the fragment of the input tuplet.
 */
tuplet_t *gs_tuplet_rewind(tuplet_t *tuplet);

void gs_tuplet_set_null(tuplet_t *tuplet);

bool gs_tuplet_is_null(tuplet_t *tuplet);



size_t gs_tuplet_size(tuplet_t *tuplet);

void *gs_update(void *dst, schema_t *frag, attr_id_t attr_id, void *src);

size_t gs_tuplet_printlen(const attr_t *attr, const void *field_data);

size_t gs_tuplet_size_by_schema(const schema_t *schema);


//
//DECLARE_TUPLET_INSERT(bool, bool, FT_BOOL)
//
//DECLARE_TUPLET_INSERT(int8, int8_t, FT_INT8)
//
//DECLARE_TUPLET_INSERT(int16, int16_t, FT_INT16)
//
//DECLARE_TUPLET_INSERT(int32, int32_t, FT_INT32)
//
//DECLARE_TUPLET_INSERT(int64, int64_t, FT_INT64)
//
//DECLARE_TUPLET_INSERT(uint8, uint8_t, FT_UINT8)
//
//DECLARE_TUPLET_INSERT(uint16, uint16_t, FT_UINT16)
//
//DECLARE_TUPLET_INSERT(uint32, uint32_t, FT_UINT32)
//
//DECLARE_TUPLET_INSERT(uint64, uint64_t, FT_UINT64)
//
//DECLARE_TUPLET_INSERT(float32, float, FT_FLOAT32)
//
//DECLARE_TUPLET_INSERT(float64, double, FT_FLOAT64)
//
//DECLARE_ARRAY_FIELD_INSERT(string, char, FT_CHAR)

size_t gs_tuplet_printlen(const attr_t *attr, const void *field_data);