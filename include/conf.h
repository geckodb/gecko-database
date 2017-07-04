#pragma once

#define NUM_INIT_ENV_VARS       10

#define CONF_ENV_VAR_MYIMDB_HOME                "MYIMDB_HOME"

#define CONF_BUILT_IN_VARNAME_MYIMDB_HOME       "${MYIMDB_HOME}"
#define CONF_BUILT_IN_VARNAME_WORKING_DIR       "${WORKING_DIR}"
#define CONF_BUILT_IN_VARNAME_BIN_DIR           "${BIN_DIR}"
#define CONF_BUILT_IN_VARNAME_ETC_DIR           "${ETC_DIR}"
#define CONF_BUILT_IN_VARNAME_SWAP_DIR          "${SWAP_DIR}"

#define CONF_VAR_MYIMDB_HOME                    "myimdb_home"

#define CONF_VAR_MYIMDB_BIN                     "myimdb_bin"
#define CONF_VAR_MYIMDB_BIN_DEFAULT_VALUE       CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/bin"

#define CONF_VAR_MYIMDB_SWAP                    "myimdb_swap"
#define CONF_VAR_MYIMDB_SWAP_DEFAULT_VALUE      CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/swap"

#define CONF_VAR_MYIMDB_ETC                     "myimdb_etc"
#define CONF_VAR_MYIMDB_ETC_DEFAULT_VALUE       CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/etc"

#define CONF_FILE_PATH_SOFTLINKS                CONF_BUILT_IN_VARNAME_WORKING_DIR "/myimdb-softlink.conf"

#define CONF_FILE_PATH_SWAPBUF_CONFIG           CONF_BUILT_IN_VARNAME_ETC_DIR "/myimdb/myimdb-swapbuf.conf"
#define CONF_SETTING_SWAP_BUFFER_HOTSTORE_LIM   "swap_buffer.hotstore.limit"

void
conf_load();

void
conf_get(
        void *      out,
        const char *module,
        const char *settings_key
);

void
conf_free();