#include <frag_printer.h>

#define IMPL_COMPLETE(printer)                                                                                         \
    (printer->_print != NULL && printer->_free != NULL)                                                                \

void gs_frag_printer_print(FILE *file, frag_printer_type_tag type, frag_t *frag, size_t row_offset, size_t limit)
{
    REQUIRE_NONNULL(file);
    REQUIRE_NONNULL(frag);

    size_t num_installed_printers = sizeof(frag_printer_register) / sizeof(frag_printer_register[0]);
    for (size_t i = 0; i < num_installed_printers; i++) {
        if (frag_printer_register[i].type == type) {
            frag_printer_t *printer = frag_printer_register[i].create();
            panic_if(!(IMPL_COMPLETE(printer)), BADINTERNAL, "fragment printer implementation is incomplete");
            printer->_print(printer, file, frag, row_offset, limit);
            printer->_free(printer);
            free (printer);
            return;
        }
    }
    panic(BADINTERNAL, "selected printer implementation is unknown");
}