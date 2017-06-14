// Several pre-defined functions related to hash operations
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
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

#include "defs.h"

typedef size_t (*hash_code_fn_t)(size_t key_size, const void *key);
typedef size_t (*hash_fn_t)(size_t code, size_t upper_bound);

// ---------------------------------------------------------------------------------------------------------------------
// H A S H   C O D E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t hash_code_size_t(size_t key_size, const void *key);

// ---------------------------------------------------------------------------------------------------------------------
// H A S H   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

size_t hash_fn_mod(size_t code, size_t upper_bound);