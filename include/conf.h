#pragma once

#define NUM_INIT_ENV_VARS       10

#define CONF_ENV_VAR_MYIMDB_HOME              "MYIMDB_HOME"

#define CONF_BUILT_IN_VARNAME_MYIMDB_HOME     "${MYIMDB_HOME}"
#define CONF_BUILT_IN_VARNAME_WORKING_DIR     "${WORKING_DIR}"

#define CONF_VAR_MYIMDB_HOME                  "myimdb_home"

#define CONF_VAR_MYIMDB_BIN                   "myimdb_bin"
#define CONF_VAR_MYIMDB_BIN_DEFAULT_VALUE     CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/bin"

#define CONF_VAR_MYIMDB_SWAP                  "myimdb_swap"
#define CONF_VAR_MYIMDB_SWAP_DEFAULT_VALUE    CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/swap"

#define CONF_VAR_MYIMDB_ETC                   "myimdb_etc"
#define CONF_VAR_MYIMDB_ETC_DEFAULT_VALUE     CONF_BUILT_IN_VARNAME_MYIMDB_HOME "/etc"

#define CONF_FILE_PATH_SOFTLINKS              CONF_BUILT_IN_VARNAME_WORKING_DIR "/myimdb-softlink.conf"

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