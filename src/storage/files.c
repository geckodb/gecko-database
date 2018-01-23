#include <storage/files.h>

gs_status_t files_exists(const char * filename) {
    FILE *file;
    if ((file = fopen(filename, "r"))) {
        fclose(file);
        return GS_TRUE;
    }
    return GS_FALSE;
}