#pragma once

#include <tuplet.h>

struct FIELD {
    TUPLET *tuplet; /*<! tuplet to which this field belongs to */

    ATTR_ID attr_id; /*<! current attribute id to which this tuplet is seeked */
    void *field; /*<! pointer in data in 'tuplet_base' of field content of attribute 'attr_id */

    /* operations */
    void (*_next)(struct FIELD *self); /* seeks to the next field inside this tuplet */
    void *(*_read)(struct FIELD *self); /* access current field 'attr_id' of this tuplet */
    void (*_update)(struct FIELD *self, const void *data); /* update data of current field 'attr_id' */
    void (*_set_null)(struct FIELD *self); /* set the field for 'attr_id' to NULL */
    bool (*_is_null)(struct FIELD *self); /* returns true iff field of 'attr_id' is null */
    void (*_close)(struct FIELD *self); /* close request for this field */
};

void gs_field_next(struct FIELD *field);

void *gs_field_read(struct FIELD *field);

void gs_field_update(struct FIELD *field, const void *data);

void gs_field_set_null(struct FIELD *field);

bool gs_field_is_null(struct FIELD *field);

void gs_field_close(struct FIELD *field);

void gs_field_close(struct FIELD *field);

struct FIELD *gs_field_open(struct TUPLET *tuplet);