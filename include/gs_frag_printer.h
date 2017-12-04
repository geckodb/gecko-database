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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gecko-commons/gecko-commons.h>
#include <gs_frag.h>
#include <frag_printers/gs_console_printer.h>

// ---------------------------------------------------------------------------------------------------------------------
// F O R W A R D   D E C L A R A T I O N S
// ---------------------------------------------------------------------------------------------------------------------

struct gs_frag_t;

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum gs_frag_printer_type_tag_e {
    FPTT_CONSOLE_PRINTER
} gs_frag_printer_type_tag_e;

typedef struct gs_frag_printer_t {
    void (*_print)(struct gs_frag_printer_t *self, FILE *file, struct gs_frag_t *frag, size_t row_offset, size_t limit);
    void (*_free)(struct gs_frag_printer_t *self);
    gs_frag_printer_type_tag_e tag;
    void *extra;
} gs_frag_printer_t;

static struct gs_frag_printer_register_entry_t {
    gs_frag_printer_type_tag_e type;
    gs_frag_printer_t *(*create)();
} frag_printer_register[] = {
    { FPTT_CONSOLE_PRINTER, gs_console_printer_new }
};

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void gs_frag_printer_print(FILE *file, gs_frag_printer_type_tag_e type, struct gs_frag_t *frag, size_t row_offset,
                           size_t limit);

