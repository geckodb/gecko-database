#ifndef GECKO_DIRS_H
#define GECKO_DIRS_H

#include <gecko-commons/gecko-commons.h>

#define DIRS_PATH_SEP_CHAR '/'
#define DIRS_PATH_SEP_STR  "/"

gs_status_t dirs_exists(const char *path);

gs_status_t dirs_add_filename(const char **result, const char *path, const char *file, apr_pool_t *pool);

#endif
