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

#include <stdinc.h>
#include <containers/dict.h>
#include <containers/dicts/hash_table.h>
#include <conf.h>
#include <utils.h>


#define BADCWD          "Unable to get working directory (file name might exceed limit of %d)"
#define NOCONFIG        "Configuration map for '%s' is not set. Did you call 'conf_load'?"

static inline char
path_separator()
{
#ifdef PLATFORM_WINDOWS
    return '\\';
#else
    return '/';
#endif
}

// Taken from https://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
static char *replace(const char *in, const char *pattern, const char *by)
{
    size_t outsize = strlen(in) + 1;
    // TODO maybe avoid reallocing by counting the non-overlapping occurences of pattern
    char *res = REQUIRE_MALLOC(outsize);
    // use this to iterate over the output
    size_t resoffset = 0;

    char *needle;
    while ((needle = strstr(in, pattern))) {
        // copy everything up to the pattern
        memcpy(res + resoffset, in, needle - in);
        resoffset += needle - in;

        // skip the pattern in the input-string
        in = needle + strlen(pattern);

        // adjust space for replacement
        outsize = outsize - strlen(pattern) + strlen(by);
        res = realloc(res, outsize);

        // copy the pattern
        memcpy(res + resoffset, by, strlen(by));
        resoffset += strlen(by);
    }

    // copy the remaining input
    strcpy(res + resoffset, in);

    return res;
}

static inline void
get_home_dir(
    dict_t *    env)
{
    char *      myimdb_home          = NULL;
    const char *myimdb_home_path     = NULL;
    const char *env_myimdb_home      = getenv(CONF_ENV_VAR_MYIMDB_HOME);
    myimdb_home                      = REQUIRE_MALLOC(strlen(CONF_VAR_MYIMDB_HOME) + 1);
    strcpy(myimdb_home, CONF_VAR_MYIMDB_HOME);

    if (env_myimdb_home) {
        myimdb_home_path = env_myimdb_home;
    } else {
        char    buffer[FILENAME_MAX];
        char *  parent_dir          = NULL;
        myimdb_home_path            = getcwd(buffer, sizeof(buffer));
        panic_if((myimdb_home_path == NULL),
                 BADCWD, FILENAME_MAX);
        if ((parent_dir = strrchr(buffer, path_separator())))
            *parent_dir = '\0';
    }

    char *val = strdup(myimdb_home_path);
    dict_put(env, &myimdb_home, &val);

}

const char *
get_by_key(
        dict_t *dict,
        const char *key_name)
{
    const char *import_name = strdup(key_name);
    const void *result = dict_get(dict, &import_name);
    return (result != NULL ? *(char **) result : NULL);
}

static inline void
register_path_dir(
    dict_t *    env,
    const char *key_name,
    const char *value)
{
    char *      import_name  = strdup(key_name);
    char *      import_value = strdup(value);
    dict_put(env, &import_name, &import_value);
}

char *resolve_variable(
    dict_t *env,
    char *string,
    const char *var_name,
    const char *built_in_var_name
)
{
    const char *value = get_by_key(env, var_name);
    char *result = replace(string, built_in_var_name, value);
    free (string);
    return result;
}

char *resolve_variables(
    dict_t *env,
    const char *string)
{
    char        buffer[FILENAME_MAX];
    char *      myimdb_cwd    = getcwd(buffer, sizeof(buffer));
    panic_if ((myimdb_cwd == NULL), BADCWD, FILENAME_MAX);

    char *str = replace(string, CONF_BUILT_IN_VARNAME_WORKING_DIR, myimdb_cwd);

    str = resolve_variable(env, str, CONF_VAR_MYIMDB_HOME, CONF_BUILT_IN_VARNAME_MYIMDB_HOME);
    str = resolve_variable(env, str, CONF_VAR_MYIMDB_BIN,  CONF_BUILT_IN_VARNAME_BIN_DIR);
    str = resolve_variable(env, str, CONF_VAR_MYIMDB_ETC,  CONF_BUILT_IN_VARNAME_ETC_DIR);
    str = resolve_variable(env, str, CONF_VAR_MYIMDB_SWAP, CONF_BUILT_IN_VARNAME_SWAP_DIR);

    return str;
}

void
update_dir(dict_t *env, pref_t *pref, const char *var_name, const char *key_name, const char *default_value)
{
    char *path = resolve_variables(env, pref_get_str(pref, key_name, default_value));
    register_path_dir(env, var_name,  path);
    free (path);
}

pref_t swapbuf_config;

void
load_config_file(
    dict_t *env,
    pref_t *config,
    const char *file_path)
{
    char *config_file = resolve_variables(env, file_path);
    pref_load(config, config_file, env, resolve_variables);
    free (config_file);
}

dict_t *conf_load()
{
    dict_t *env = hash_table_new_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                    sizeof(char *), sizeof(char *), NUM_INIT_ENV_VARS, NUM_INIT_ENV_VARS, 1.7f, 0.75f,
                                    str_equals, str_str_clean_up, true);

    get_home_dir(env);

    char *bin_path = resolve_variables(env, CONF_VAR_MYIMDB_BIN_DEFAULT_VALUE);
    register_path_dir(env, CONF_VAR_MYIMDB_BIN, bin_path);

    char *etc_path = resolve_variables(env, CONF_VAR_MYIMDB_ETC_DEFAULT_VALUE);
    register_path_dir(env, CONF_VAR_MYIMDB_ETC,  etc_path);

    char *swap_path = resolve_variables(env, CONF_VAR_MYIMDB_SWAP_DEFAULT_VALUE);
    register_path_dir(env, CONF_VAR_MYIMDB_SWAP, swap_path);

    /* change the paths by an optional config-file in home directory */
    char *soft_link_file = resolve_variables(env, CONF_FILE_PATH_SOFTLINKS);
    if ((access(soft_link_file, F_OK ) != -1)) {
        pref_t soft_link_config;
        pref_load(&soft_link_config, soft_link_file, NULL, NULL);

        update_dir(env, &soft_link_config, CONF_VAR_MYIMDB_BIN, "bin_path", bin_path);
        update_dir(env, &soft_link_config, CONF_VAR_MYIMDB_ETC, "etc_path", etc_path);
        update_dir(env, &soft_link_config, CONF_VAR_MYIMDB_SWAP, "swap_path", swap_path);

       // pref_free(&soft_link_config); // TODO: freeing up pref_delete causes freeing up unallocated pointer
    }

    free (bin_path);
    free (etc_path);
    free (swap_path);

    load_config_file(env, &swapbuf_config, CONF_FILE_PATH_SWAPBUF_CONFIG);

    printf("%s: %s\n", CONF_VAR_MYIMDB_HOME, get_by_key(env, CONF_VAR_MYIMDB_HOME));
    printf("%s: %s\n", CONF_VAR_MYIMDB_BIN,  get_by_key(env, CONF_VAR_MYIMDB_BIN));
    printf("%s: %s\n", CONF_VAR_MYIMDB_ETC,  get_by_key(env, CONF_VAR_MYIMDB_ETC));
    printf("%s: %s\n", CONF_VAR_MYIMDB_SWAP,  get_by_key(env, CONF_VAR_MYIMDB_SWAP));

    printf("%s: %s\n", CONF_SETTING_SWAP_BUFFER_HOTSTORE_LIM,     pref_get_str(&swapbuf_config, CONF_SETTING_SWAP_BUFFER_HOTSTORE_LIM, "some limit"));
    printf("%s: %s\n", CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_DEF,    pref_get_str(&swapbuf_config, CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_DEF, "some min"));
    printf("%s: %s\n", CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_MAX,    pref_get_str(&swapbuf_config, CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_MAX, "some max"));
    printf("%s: %s\n", CONF_SETTING_SWAP_BUFFER_PAGE_DEF_MSPACE,  pref_get_str(&swapbuf_config, CONF_SETTING_SWAP_BUFFER_PAGE_DEF_MSPACE, "some space"));
    printf("%s: %s\n", CONF_SETTING_SWAP_BUFFER_PAGE_HUGE_MSPACE, pref_get_str(&swapbuf_config, CONF_SETTING_SWAP_BUFFER_PAGE_HUGE_MSPACE, "huge"));
    printf("%s: %s\n", CONF_SETTING_SWAP_BUFFER_MMAP_SWAP_DIR,    pref_get_str(&swapbuf_config, CONF_SETTING_SWAP_BUFFER_MMAP_SWAP_DIR, "a dir"));

    return env;
}

void conf_free(struct dict_t * conf)
{
    hash_table_delete(conf);
}

const char *
conf_get_str(
        const char *settings_key,
        const char *default_value
) {
    WARN_IF((swapbuf_config.dict == NULL), NOCONFIG, settings_key);
    return pref_get_str(&swapbuf_config, settings_key, default_value);
}

void
conf_get_size_t(
        size_t *out,
        const char *settings_key,
        size_t default_value
) {
    WARN_IF((swapbuf_config.dict == NULL), NOCONFIG, settings_key);
    pref_get_size_t(out, &swapbuf_config, settings_key, &default_value);
}

void db_config_load(db_config_t *config)
{
    REQUIRE_NONNULL(config);
    config->inet.port = CONF_INET_PORT;
    config->log.debug = config->log.warn = config->log.error = true;
}
