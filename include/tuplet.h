#pragma once

#include <stdinc.h>
#include <fragment.h>

struct SCHEMA;

typedef struct TUPLET {
    struct FRAGMENT *fragment; /*<! fragment in which this tuplet exists */
    size_t id; /*<! number of this tuplet inside the fragment */

    /* operations */
    void (*_next)(struct TUPLET *self); /* seeks to the next tuplet inside this fragment */
    struct FIELD *(*_open)(struct TUPLET *self, ATTR_ID attr_id); /*<! access the field data of this tuplet */
    void (*_update)(struct TUPLET *self, const void *data); /*<! updates all fields of this tuplet and moves to next */
    void (*_set_null)(struct TUPLET *self); /*<! updates all fields of this tuplet to NULL, and moves to next */
    void (*_delete)(struct TUPLET *self); /* request to delete this tuplet from fragment */

} TUPLET;

TUPLET *gs_tuplet_open(struct FRAGMENT *frag);

TUPLET *gs_tuplet_next(TUPLET *tuplet);

void *gs_tuplet_read_unsafe(TUPLET *tuplet);

void gs_tuplet_update_unsafe(TUPLET *tuplet, const void *data);

void gs_tuplet_set_null(TUPLET *tuplet);

bool gs_tuplet_is_null(TUPLET *tuplet);

void gs_tuplet_close(TUPLET *tuplet);

void *gs_update(void *dst, SCHEMA *frag, ATTR_ID attr_id, void *src);