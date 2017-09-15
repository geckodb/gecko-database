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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <unsafe.h>
#include <tuplet_field.h>
#include <attr.h>

#define REQUIRE_INSTANCEOF_THIS(x)                                                                                     \
    REQUIRE((x->tag == FPTT_CONSOLE_PRINTER), BADTAG);

static inline void console_printer_print(struct frag_printer_t *self, FILE *file, frag_t *frag, size_t row_offset, size_t limit);
static inline void console_printer_free(struct frag_printer_t *self);

static inline void calc_field_print_lens(vec_t *field_print_lens, frag_t *frag, size_t num_attr);

static inline void print_h_line(FILE *file, const frag_t *frag, size_t num_attr, schema_t *schema, vec_t *field_print_lens);
static inline void print_frag_header(FILE *file, const frag_t *frag, vec_t *field_print_lens, size_t num_attr);
static inline void print_frag_body(FILE *file, frag_t *frag, vec_t *field_print_lens, size_t num_attr, size_t row_offset, size_t limit);

// ---------------------------------------------------------------------------------------------------------------------

struct frag_printer_t *console_printer_new()
{
    frag_printer_t *result = REQUIRE_MALLOC(sizeof(frag_printer_t));
    *result = (frag_printer_t) {
        ._print = console_printer_print,
        ._free  = console_printer_free,
        .tag    = FPTT_CONSOLE_PRINTER,
        .extra  = NULL
    };
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

static inline void calc_field_print_lens(vec_t *field_print_lens, frag_t *frag, size_t num_attr)
{
    assert (field_print_lens);

    tuplet_t tuplet;
    tuplet_open(&tuplet, frag, 0);
    size_t num_tuplets = frag->ntuplets;
    schema_t *schema = frag_schema(frag);

    while (num_tuplets--) {
        struct tuplet_field_t field;
        tuplet_field_open(&field, &tuplet);
        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
            enum field_type type = schema_attr_type(schema, attr_idx);
            const struct attr_t *attr = schema_attr_by_id(schema, attr_idx);

            size_t this_print_len_attr  = strlen(attr_name(attr));
            size_t this_print_len_value = unsafe_field_println(type, tuplet_field_read(&field));
            size_t this_print_len_field = max(this_print_len_attr, this_print_len_value);

            size_t all_print_len = *(size_t *) vec_at(field_print_lens, attr_idx);
            all_print_len = max(all_print_len, this_print_len_field);
            vec_set(field_print_lens, attr_idx, 1, &all_print_len);
            tuplet_field_next(&field, true);
        }
        tuplet_next(&tuplet);
    }
}

static inline void console_printer_print(struct frag_printer_t *self, FILE *file, frag_t *frag, size_t row_offset, size_t limit)
{
    REQUIRE_INSTANCEOF_THIS(self);
    size_t num_attr = frag_num_of_attributes(frag);
    vec_t *field_print_lens = vec_new(sizeof(size_t), num_attr + 1);
    vec_resize(field_print_lens, num_attr);
    size_t zero = 0;
    vec_memset(field_print_lens, 0, num_attr, &zero);

    calc_field_print_lens(field_print_lens, frag, num_attr);
    print_frag_header(file, frag, field_print_lens, num_attr);
    print_frag_body(file, frag, field_print_lens, num_attr, row_offset, limit);
    vec_free(field_print_lens);
}

static inline void console_printer_free(struct frag_printer_t *self)
{
    REQUIRE_INSTANCEOF_THIS(self);
}

static inline void print_h_line(FILE *file, const frag_t *frag, size_t num_attr, schema_t *schema, vec_t *field_print_lens)
{
    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t   col_width = *(size_t *) vec_at(field_print_lens, attr_idx);

        printf("+");
        for (size_t i = 0; i < col_width + 2; i++)
            printf("-");
    }

    printf("+\n");
}

static inline void print_frag_header(FILE *file, const frag_t *frag, vec_t *field_print_lens, size_t num_attr)
{
    char format_buffer[2048];
    schema_t *schema   = frag_schema(frag);

    print_h_line(file, frag, num_attr, schema, field_print_lens);

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        const struct attr_t *attr = schema_attr_by_id(schema, attr_idx);
        size_t  col_width = *(size_t *) vec_at(field_print_lens, attr_idx);
        sprintf(format_buffer, "| %%-%zus ", col_width);
        printf(format_buffer, attr_name(attr));
    }
    printf("|\n");

    print_h_line(file, frag, num_attr, schema, field_print_lens);
}

static inline void print_frag_body(FILE *file, frag_t *frag, vec_t *field_print_lens, size_t num_attr, size_t row_offset, size_t limit)
{
    assert (field_print_lens);

    char format_buffer[2048];
    tuplet_t tuplet;
    tuplet_open(&tuplet, frag, 0);
    size_t num_tuples = frag->ntuplets;
    schema_t *schema = frag_schema(frag);

    while (num_tuples--) {
        struct tuplet_field_t field;
        tuplet_field_open(&field, &tuplet);
        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
            const attr_t *attr = schema_attr_by_id(schema, attr_idx);
            char *str = unsafe_field_str(attr->type, tuplet_field_read(&field));
            size_t print_len = max(strlen(str), *(size_t *) vec_at(field_print_lens, attr_idx));
            sprintf(format_buffer, "| %%-%zus ", print_len);
            printf(format_buffer, str);
            free (str);
            tuplet_field_next(&field, false);
        }
        tuplet_next(&tuplet);
        printf("|\n");
    }

    print_h_line(file, frag, num_attr, schema, field_print_lens);
}