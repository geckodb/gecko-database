#pragma once

#include <tuplet.h>

typedef struct field_t {
    tuplet_t *tuplet; /*<! tuplet to which this field belongs to */

    attr_id_t attr_id; /*<! current attribute id to which this tuplet is seeked */
    void *field; /*<! pointer in data in 'tuplet_base' of field content of attribute 'attr_id */

    /* operations */
    struct field_t* (*_next)(struct field_t *self); /* seeks to the next field inside this tuplet */
    const void *(*_read)(struct field_t *self); /* access current field 'attr_id' of this tuplet */
    void (*_update)(struct field_t *self, const void *data); /* update data of current field 'attr_id' */
    void (*_set_null)(struct field_t *self); /* set the field for 'attr_id' to NULL */
    bool (*_is_null)(struct field_t *self); /* returns true iff field of 'attr_id' is null */
    void (*_close)(struct field_t *self); /* close request for this field */
} field_t;

field_t *gs_field_next(field_t *field);

const void *gs_field_read(field_t *field);

void gs_field_update(field_t *field, const void *data);

void gs_field_set_null(field_t *field);

bool gs_field_is_null(field_t *field);

void gs_field_close(field_t *field);

void gs_field_close(field_t *field);

field_t *gs_field_open(tuplet_t *tuplet);

size_t gs_field_size(field_t *field);

size_t get_field_size_by_id(const attr_t *attr, attr_id_t attr_idx);