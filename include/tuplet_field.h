#pragma once

#include <tuplet.h>

typedef struct tuplet_field_t {
    tuplet_t *tuplet; /*<! tuplet to which this attr_value_ptr belongs to */

    attr_id_t attr_id; /*<! current attribute tuplet_id to which this tuplet is seeked */
    void *attr_value_ptr; /*<! pointer in data in 'tuplet_base' of attr_value_ptr content of attribute 'attr_id */

    /* operations */
    bool (*_next)(struct tuplet_field_t *self); /* seeks to the next attr_value_ptr inside this tuplet */
    bool (*_seek)(struct tuplet_field_t *self, attr_id_t attr_id); /* seeks the attr_value_ptr to this attribute */
    const void *(*_read)(struct tuplet_field_t *self); /* access current attr_value_ptr 'attr_id' of this tuplet */
    void (*_update)(struct tuplet_field_t *self, const void *data); /* update data of current attr_value_ptr 'attr_id' */
    void (*_set_null)(struct tuplet_field_t *self); /* set the attr_value_ptr for 'attr_id' to NULL */
    bool (*_is_null)(struct tuplet_field_t *self); /* returns true iff attr_value_ptr of 'attr_id' is null */
    void (*_close)(struct tuplet_field_t *self); /* close request for this attr_value_ptr */
} tuplet_field_t;

tuplet_field_t *gs_tuplet_field_open(tuplet_t *tuplet);

tuplet_field_t *gs_tuplet_field_seek(tuplet_t *tuplet, attr_id_t attr_id);

bool gs_tuplet_field_next(tuplet_field_t *field);

const void *gs_tuplet_field_read(tuplet_field_t *field);

void gs_tuplet_field_update(tuplet_field_t *field, const void *data);

bool gs_tuplet_field_write(tuplet_field_t *field, const void *data);

bool gs_tuplet_field_write_eval(tuplet_field_t *field, bool eval);

void gs_tuplet_field_set_null(tuplet_field_t *field);

bool gs_tuplet_field_is_null(tuplet_field_t *field);

void gs_tuplet_field_close(tuplet_field_t *field);

size_t gs_tuplet_field_size(tuplet_field_t *field);

size_t gs_attr_total_size(const struct attr_t *attr);

size_t gs_tuplet_field_get_printlen(const tuplet_field_t *field);

enum field_type gs_tuplet_field_get_type(const tuplet_field_t *field);

char *gs_tuplet_field_to_string(const tuplet_field_t *field);