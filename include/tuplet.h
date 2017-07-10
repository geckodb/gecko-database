#pragma once

#include <stdinc.h>
#include <fragment.h>

struct schema_t;

typedef struct tuplet_t {
    struct fragment_t *fragment; /*<! fragment in which this tuplet exists */
    size_t id; /*<! number of this tuplet inside the fragment */

    /* operations */
    void (*_next)(struct tuplet_t *self); /* seeks to the next tuplet inside this fragment */
    struct field_t *(*_open)(struct tuplet_t *self, ATTR_ID attr_id); /*<! access the field data of this tuplet */
    void (*_update)(struct tuplet_t *self, const void *data); /*<! updates all fields of this tuplet and moves to next */
    void (*_set_null)(struct tuplet_t *self); /*<! updates all fields of this tuplet to NULL, and moves to next */
    void (*_delete)(struct tuplet_t *self); /* request to delete this tuplet from fragment */

} tuplet_t;

tuplet_t *gs_tuplet_open(struct fragment_t *frag);

tuplet_t *gs_tuplet_next(tuplet_t *tuplet);

void *gs_tuplet_read_unsafe(tuplet_t *tuplet);

void gs_tuplet_update_unsafe(tuplet_t *tuplet, const void *data);

void gs_tuplet_set_null(tuplet_t *tuplet);

bool gs_tuplet_is_null(tuplet_t *tuplet);

void gs_tuplet_close(tuplet_t *tuplet);

void *gs_update(void *dst, schema_t *frag, ATTR_ID attr_id, void *src);