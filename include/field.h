#pragma once

#include <tuplet.h>

typedef struct field_t {
    tuplet_t *tuplet; /*<! tuplet to which this attr_value_ptr belongs to */

    attr_id_t attr_id; /*<! current attribute slot_id to which this tuplet is seeked */
    void *attr_value_ptr; /*<! pointer in data in 'tuplet_base' of attr_value_ptr content of attribute 'attr_id */

    /* operations */
    bool (*_next)(struct field_t *self); /* seeks to the next attr_value_ptr inside this tuplet */
    const void *(*_read)(struct field_t *self); /* access current attr_value_ptr 'attr_id' of this tuplet */
    void (*_update)(struct field_t *self, const void *data); /* update data of current attr_value_ptr 'attr_id' */
    void (*_set_null)(struct field_t *self); /* set the attr_value_ptr for 'attr_id' to NULL */
    bool (*_is_null)(struct field_t *self); /* returns true iff attr_value_ptr of 'attr_id' is null */
    void (*_close)(struct field_t *self); /* close request for this attr_value_ptr */
} field_t;

bool gs_field_next(field_t *field);

const void *gs_field_read(field_t *field);

void gs_field_update(field_t *field, const void *data);

bool gs_field_write(field_t *field, const void *data);

void gs_field_set_null(field_t *field);

bool gs_field_is_null(field_t *field);

void gs_field_close(field_t *field);

void gs_field_close(field_t *field);

field_t *gs_field_open(tuplet_t *tuplet);

size_t gs_field_size(field_t *field);

size_t gs_attr_total_size(const attr_t *attr);

size_t gs_field_get_printlen(const field_t *field);

enum field_type gs_field_get_type(const field_t *field);

char *gs_field_to_string(const field_t *field);