#include <frag_printers/console_printer.h>
#include <unsafe.h>
#include <field.h>
#include <attr.h>

#define REQUIRE_INSTANCEOF_THIS(x)                                                                                     \
    require((x->tag == FPTT_CONSOLE_PRINTER), BADTAG);

static inline void console_printer_print(struct frag_printer_t *self, FILE *file, frag_t *frag, size_t row_offset, size_t limit);
static inline void console_printer_free(struct frag_printer_t *self);

static inline void calc_field_print_lens(vector_t *field_print_lens, frag_t *frag, size_t num_attr);

static inline void print_h_line(FILE *file, const frag_t *frag, size_t num_attr, schema_t *schema, vector_t *field_print_lens);
static inline void print_frag_header(FILE *file, const frag_t *frag, vector_t *field_print_lens, size_t num_attr);
static inline void print_frag_body(FILE *file, frag_t *frag, vector_t *field_print_lens, size_t num_attr);

// ---------------------------------------------------------------------------------------------------------------------

struct frag_printer_t *console_printer_create()
{
    frag_printer_t *result = require_good_malloc(sizeof(frag_printer_t));
    *result = (frag_printer_t) {
        ._print = console_printer_print,
        ._free  = console_printer_free,
        .tag    = FPTT_CONSOLE_PRINTER,
        .extra  = NULL
    };
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

static inline void calc_field_print_lens(vector_t *field_print_lens, frag_t *frag, size_t num_attr)
{
    assert (field_print_lens);

    tuplet_t *tuplet = gs_tuplet_open(frag);
    bool not_eof     = (tuplet != NULL);
    schema_t *schema = gs_fragment_get_schema(frag);

    while (not_eof) {
        struct field_t *field = gs_field_open(tuplet);
        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
            enum field_type type = gs_schema_attr_type(schema, attr_idx);
            attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);

            size_t this_print_len_attr  = strlen(gs_attr_get_name(attr));
            size_t this_print_len_value = gs_unsafe_field_get_println(type, gs_field_read(field));
            size_t this_print_len_field = max(this_print_len_attr, this_print_len_value);

            size_t all_print_len = *(size_t *) vector_at(field_print_lens, attr_idx);
            all_print_len = max(all_print_len, this_print_len_field);
            vector_set(field_print_lens, attr_idx, 1, &all_print_len);
            gs_field_next(field);
        }
        not_eof = gs_tuplet_next(tuplet);
    }
}

static inline void console_printer_print(struct frag_printer_t *self, FILE *file, frag_t *frag, size_t row_offset, size_t limit)
{
    REQUIRE_INSTANCEOF_THIS(self);
    size_t num_attr = gs_fragment_num_of_attributes(frag);
    vector_t *field_print_lens = vector_create(sizeof(size_t), num_attr + 1);
    vector_resize(field_print_lens, num_attr);
    vector_memset(field_print_lens, 0, num_attr, 0);

    calc_field_print_lens(field_print_lens, frag, num_attr);
    print_frag_header(file, frag, field_print_lens, num_attr);
    print_frag_body(file, frag, field_print_lens, num_attr);
    vector_free(field_print_lens);
}

static inline void console_printer_free(struct frag_printer_t *self)
{
    REQUIRE_INSTANCEOF_THIS(self);
}

static inline void print_h_line(FILE *file, const frag_t *frag, size_t num_attr, schema_t *schema, vector_t *field_print_lens)
{
    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t   col_width = *(size_t *)vector_at(field_print_lens, attr_idx);

        printf("+");
        for (size_t i = 0; i < col_width + 2; i++)
            printf("-");
    }

    printf("+\n");
}

static inline void print_frag_header(FILE *file, const frag_t *frag, vector_t *field_print_lens, size_t num_attr)
{
    char format_buffer[2048];
    schema_t *schema   = gs_fragment_get_schema(frag);

    print_h_line(file, frag, num_attr, schema, field_print_lens);

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
        size_t  col_width = *(size_t *) vector_at(field_print_lens, attr_idx);
        sprintf(format_buffer, "| %%-%zus ", col_width);
        printf(format_buffer, gs_attr_get_name(attr));
    }
    printf("|\n");

    print_h_line(file, frag, num_attr, schema, field_print_lens);
}

static inline void print_frag_body(FILE *file, frag_t *frag, vector_t *field_print_lens, size_t num_attr)
{
    assert (field_print_lens);

    char format_buffer[2048];
    tuplet_t *tuplet = gs_tuplet_open(frag);
    bool not_eof     = (tuplet != NULL);
    schema_t *schema = gs_fragment_get_schema(frag);

    while (not_eof) {
        struct field_t *field = gs_field_open(tuplet);
        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
            attr_t *attr = gs_schema_attr_by_id(schema, attr_idx);
            char *str = gs_unsafe_field_to_string(attr->type, gs_field_read(field));
            size_t print_len = max(strlen(str), *(size_t *) vector_at(field_print_lens, attr_idx));
            sprintf(format_buffer, "| %%-%zus ", print_len);
            printf(format_buffer, str);
            free (str);
            gs_field_next(field);
        }
        printf("|\n");
        not_eof = gs_tuplet_next(tuplet);
    }
}