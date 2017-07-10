// An implementation of a tables attribute data structure
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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

#include <defs.h>
#include <types.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    attribute_autoinc = 1 << 1,
    attribute_primary = 1 << 2,
    attribute_nonnull = 1 << 3,
    attribute_unique  = 1 << 4
} attribute_flags;

typedef struct {
    unsigned short non_null    : 1;
    unsigned short primary_key : 1;
    unsigned short auto_inc    : 1;
    unsigned short unique      : 1;
} attribute_flags_t;

typedef struct {
    type type;
    size_t length;
    const char *name;
    attribute_flags_t flags;
} attribute_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

attribute_t *attribute_create(type type, size_t length, const char *name, attribute_flags flags);

attribute_t *attribute_cpy(const attribute_t *proto);

attribute_flags_t attribute_default_flags();

attribute_flags attribute_flags_t_to_flag(attribute_flags_t flags);

attribute_flags_t attribute_flags_to_flag_t(attribute_flags flags);

bool attribute_free(attribute_t *attr);

bool attribute_comp(const attribute_t *lhs, const attribute_t *rhs);

void attribute_print(FILE *file, const attribute_t *attr);

void attribute_flags_print(FILE *file, attribute_flags_t flags);
