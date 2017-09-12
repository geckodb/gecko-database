#include <unsafe.h>
#include <frag.h>
#include <grid.h>

size_t gs_unsafe_field_get_println(enum field_type type, const void *data)
{
    char *buffer = gs_unsafe_field_to_string(type, data);
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

char *gs_unsafe_field_to_string(enum field_type type, const void *data)
{
    const size_t INIT_BUFFER_LEN = 2048;
    char *buffer = REQUIRE_MALLOC(INIT_BUFFER_LEN);
    switch (type) {
        case FT_BOOL:
            strcpy (buffer, *((bool *) data) == true? "true" : "false");
            break;
        case FT_INT8:
            sprintf(buffer, "%d", *(INT8 *) data);
            break;
        case FT_INT16:
            sprintf(buffer, "%d", *(INT16 *) data);
            break;
        case FT_INT32:
            sprintf(buffer, "%d", *(INT32 *) data);
            break;
        case FT_INT64:
            sprintf(buffer, "%lld", *(INT64 *) data);
            break;
        case FT_UINT8:
            sprintf(buffer, "%u", *(UINT8 *) data);
            break;
        case FT_UINT16:
            sprintf(buffer, "%u", *(UINT16 *) data);
            break;
        case FT_UINT32:
            sprintf(buffer, "%u", *(UINT32 *) data);
            break;
        case FT_UINT64:
            sprintf(buffer, "%llu", *(UINT64 *) data);
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
            sprintf(buffer, "%llu", *(ATTRID *) data);
            break;
        case FT_GRIDID:
            sprintf(buffer, "%zu", *(GRIDID *) data);
            break;
        case FT_TUPLEID:
            sprintf(buffer, "%u", *(TUPLEID *) data);
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
        default:
            perror("Unknown type");
            abort();
    }
    return buffer;
}