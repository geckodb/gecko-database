#pragma once

#include <stdinc.h>
#include <containers/dict.h>

#define NUM_INIT_ENV_VARS       10

#define CONF_ENV_VAR_MYIMDB_HOME                "MYIMDB_HOME"

#define CONF_BUILT_IN_VARNAME_MYIMDB_HOME           "${MYIMDB_HOME}"
#define CONF_BUILT_IN_VARNAME_WORKING_DIR           "${WORKING_DIR}"
#define CONF_BUILT_IN_VARNAME_BIN_DIR               "${BIN_DIR}"
#define CONF_BUILT_IN_VARNAME_ETC_DIR               "${ETC_DIR}"
#define CONF_BUILT_IN_VARNAME_SWAP_DIR              "${SWAP_DIR}"

#define CONF_VAR_MYIMDB_HOME                        "myimdb_home"

#define CONF_VAR_MYIMDB_BIN                         "myimdb_bin"
#define CONF_VAR_MYIMDB_BIN_DEFAULT_VALUE           CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/bin"

#define CONF_VAR_MYIMDB_SWAP                        "myimdb_swap"
#define CONF_VAR_MYIMDB_SWAP_DEFAULT_VALUE          CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/swap"

#define CONF_VAR_MYIMDB_ETC                         "myimdb_etc"
#define CONF_VAR_MYIMDB_ETC_DEFAULT_VALUE           CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/etc"

#define CONF_FILE_PATH_SOFTLINKS                    CONF_BUILT_IN_VARNAME_WORKING_DIR "/myimdb-softlink.conf"

#define CONF_FILE_PATH_SWAPBUF_CONFIG               CONF_BUILT_IN_VARNAME_ETC_DIR "/myimdb/myimdb-swapbuf.conf"
#define CONF_SETTING_SWAP_BUFFER_HOTSTORE_LIM       "swap_buffer.hotstore.limit"
#define CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_DEF      "swap_buffer.page.default_size"
#define CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_MAX      "swap_buffer.page.max_size"
#define CONF_SETTING_SWAP_BUFFER_PAGE_DEF_MSPACE    "swap_buffer.page.default_mspace"
#define CONF_SETTING_SWAP_BUFFER_PAGE_HUGE_MSPACE   "swap_buffer.page.huge_mspace"
#define CONF_SETTING_SWAP_BUFFER_MMAP_SWAP_DIR      "swap_buffer.mmap.swap_dir"

struct dict_t *conf_load();
void conf_free(struct dict_t * conf);

const char *
conf_get_str(
        const char *settings_key,
        const char *default_value
);

void
conf_get_size_t(
        size_t *out,
        const char *settings_key,
        size_t default_value
);

typedef struct db_config_t {
    struct {
        uint32_t port;
    } inet;
    struct {
        bool debug;
        bool warn;
        bool error;
    } log;
} db_config_t;

#define CONF_INET_PORT          1316

void gs_db_config_load(db_config_t *config);
