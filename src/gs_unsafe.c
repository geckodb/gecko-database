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

#include <gs_unsafe.h>
#include <gs_frag.h>
#include <gs_grid.h>
#include <inttypes.h>
#include <gs_reltype.h>

size_t gs_unsafe_field_println(enum gs_field_type_e type, const void *data)
{
    char *buffer = gs_unsafe_field_str(type, data);
    size_t print_len = strlen(buffer);
    free (buffer);
    return print_len;
}

char *clip(const char *str, size_t max_len)
{
    char *out = malloc(max_len);
    memset(out, 32, strlen(str));
    strcpy(out, str);
    out[max_len - 1] = out[max_len - 2] = out[max_len - 3] = '.';
    return out;
}

char *gs_unsafe_field_str(enum gs_field_type_e type, const void *data)
{
    const size_t INIT_BUFFER_LEN = 2048;
    char *buffer = GS_REQUIRE_MALLOC(INIT_BUFFER_LEN);
    switch (type) {
        case FT_BOOL:
            strcpy (buffer, *((bool *) data) == true? "true" : "false");
            break;
        case FT_INT8:
            sprintf(buffer, "%"PRId32, *(INT8 *) data);
            break;
        case FT_INT16:
            sprintf(buffer, "%"PRId32, *(INT16 *) data);
            break;
        case FT_INT32:
            sprintf(buffer, "%"PRId32, *(INT32 *) data);
            break;
        case FT_INT64:
            sprintf(buffer,"%"PRIu64, *(INT64 *) data);
            break;
        case FT_UINT8:
            sprintf(buffer, "%"PRIu32, *(UINT8 *) data);
            break;
        case FT_UINT16:
            sprintf(buffer, "%"PRIu32, *(UINT16 *) data);
            break;
        case FT_UINT32:
            sprintf(buffer, "%"PRIu32, *(UINT32 *) data);
            break;
        case FT_UINT64:
            sprintf(buffer, "%"PRIu64, *(UINT64 *) data);
            break;
        case FT_FLOAT32:
            sprintf(buffer, "%f", *(FLOAT32 *) data);
            break;
        case FT_FLOAT64:
            sprintf(buffer, "%f", *(FLOAT64 *) data);
            break;
        case FT_CHAR:
            if (strlen(data) + 1 > INIT_BUFFER_LEN) {
                buffer = realloc(buffer, strlen(data) + 1);
            }
            strcpy(buffer, data);
            break;
            /* internal */
        case FT_STRPTR: {
            char *output = (strlen((STRPTR) data) + 1) > INIT_BUFFER_LEN ? clip(data, INIT_BUFFER_LEN) : strdup(data);
            sprintf(buffer, "%s", output);
            free(output);
        } break;
        case FT_ATTRID:
            sprintf(buffer, "%"PRIu64, *(ATTRID *) data);
            break;
        case FT_GRIDID:
            sprintf(buffer, "%zu", *(GRIDID *) data);
            break;
        case FT_TUPLEID:
            sprintf(buffer, "%"PRIu32, *(TUPLEID *) data);
            break;
        case FT_FRAGTYPE:
            sprintf(buffer, "%s", gs_frag_str(*(FRAGTYPE *) data));
            break;
        case FT_SIZE:
            sprintf(buffer, "%zu", *(SIZE *) data);
            break;
        case FT_TFORMAT:
            sprintf(buffer, "%s", gs_tuplet_format_str(*(TFORMAT *) data));
            break;
        case FT_RELTYPE:
            sprintf(buffer, "%s", gs_reltype_str(*(RELTYPE *) data));
        default:
            perror("Unknown type");
            abort();
    }
    return buffer;
}