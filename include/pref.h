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

#include <gs.h>
#include <containers/dict.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#define NUM_INIT_CONFIG_STATEMENTS    10

// ---------------------------------------------------------------------------------------------------------------------
// D A T A   T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
    time_t last_read;
    dict_t *dict;
} pref_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void pref_load(pref_t *pref, const char *file, dict_t *dict,
        char * (*resolve_variables)(dict_t *dict, const char *string));
void pref_delete(pref_t *pref);
bool pref_has(const pref_t *pref, const char *key);
const char *pref_get_str(const pref_t *pref, const char *key, const char *default_val);
void pref_get_size_t(size_t *out, const pref_t *pref, const char *key, size_t *default_val);
