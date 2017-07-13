#pragma once

#include <stdinc.h>
#include <frag.h>
#include <frag_printers/console_printer.h>

struct frag_t;

typedef enum frag_printer_type_tag {
    FPTT_CONSOLE_PRINTER
} frag_printer_type_tag;

typedef struct frag_printer_t {
    void (*_print)(struct frag_printer_t *self, FILE *file, struct frag_t *frag, size_t row_offset, size_t limit);
    void (*_free)(struct frag_printer_t *self);
    frag_printer_type_tag tag;
    void *extra;
} frag_printer_t;

static struct frag_printer_register_entry {
    frag_printer_type_tag type;
    frag_printer_t *(*create)();
} frag_printer_register[] = {
    { FPTT_CONSOLE_PRINTER, console_printer_create }
};

void gs_frag_printer_print(FILE *file, frag_printer_type_tag type, struct frag_t *frag, size_t row_offset, size_t limit);

