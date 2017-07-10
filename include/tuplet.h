#pragma once

#include <stdinc.h>
#include <frag.h>

struct schema_t;

typedef struct tuplet_t {
    struct fragment_t *fragment; /*<! fragment in which this tuplet exists */
    size_t id; /*<! number of this tuplet inside the fragment */
    void *attr_base; /*<! pointer in fragment where first attribute of this tuplet is located */

    /* operations */
    struct tuplet_t *(*_next)(struct tuplet_t *self); /* seeks to the next tuplet inside this fragment */
    void (*_close)(struct tuplet_t *self); /* frees resources of this tuplet */
    struct field_t *(*_open)(struct tuplet_t *self); /*<! access the field data of this tuplet */
    void (*_update)(struct tuplet_t *self, const void *data); /*<! updates all fields of this tuplet and moves to next */
    void (*_set_null)(struct tuplet_t *self); /*<! updates all fields of this tuplet to NULL, and moves to next */
    void (*_delete)(struct tuplet_t *self); /* request to delete this tuplet from fragment */
    bool (*_is_null)(struct tuplet_t *self); /*<! checks whether this tuplet is NULL entirely */

} tuplet_t;

tuplet_t *gs_tuplet_open(struct fragment_t *frag);

tuplet_t *gs_tuplet_next(tuplet_t *tuplet);

tuplet_t *gs_tuplet_rewind(tuplet_t *tuplet);

void gs_tuplet_set_null(tuplet_t *tuplet);

bool gs_tuplet_is_null(tuplet_t *tuplet);

void gs_tuplet_close(tuplet_t *tuplet);

size_t gs_tuplet_size(tuplet_t *tuplet);

void *gs_update(void *dst, schema_t *frag, attr_id_t attr_id, void *src);

size_t gs_tuplet_printlen(const attr_t *attr, const void *field_data);

size_t gs_tuplet_size_by_schema(const schema_t *schema);