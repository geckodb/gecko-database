#include <storage/dirs.h>
#include <apr_strings.h>

gs_status_t dirs_exists(const char *path) {
    struct stat sb;
    return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) ? GS_TRUE : GS_FALSE;
}

gs_status_t dirs_add_filename(const char **result, const char *path, const char *file, apr_pool_t *pool)
{
    bool end_sep = path[strlen(path)] == DIRS_PATH_SEP_CHAR;
    *result = end_sep ? apr_pstrcat(pool, path, file, NULL) : apr_pstrcat(pool, path, DIRS_PATH_SEP_STR, file, NULL);
    return GS_SUCCESS;
}