#include <conf.h>
#include <pref.h>
#include <zconf.h>
#include <error.h>
#include <hash.h>
#include <containers/dictionaries/fixed_linear_hash_table.h>

#define BADCWD          "Unable to get working directory (file name might exceed limit of %d)"

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
    char *res = malloc(outsize);
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
    myimdb_home                      = malloc(strlen(CONF_VAR_MYIMDB_HOME) + 1);
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

char *resolve_variables(
    dict_t *env,
    const char *string)
{
    char        buffer[FILENAME_MAX];
    char *      myimdb_cwd    = getcwd(buffer, sizeof(buffer));
    panic_if ((myimdb_cwd == NULL), BADCWD, FILENAME_MAX);

    char *str = replace(string, CONF_BUILT_IN_VARNAME_WORKING_DIR, myimdb_cwd);

    const char *myimdb_home = get_by_key(env, CONF_VAR_MYIMDB_HOME);

    char *str2 = replace(str, CONF_BUILT_IN_VARNAME_MYIMDB_HOME, myimdb_home);

    free (str);

    return str2;
}

void
update_dir(dict_t *env, pref_t *pref, const char *var_name, const char *key_name, const char *default_value)
{
    char *path = resolve_variables(env, pref_get_str(pref, key_name, default_value));
    register_path_dir(env, var_name,  path);
    free (path);
}

void
conf_load()
{
    dict_t *env = hash_table_create_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                       sizeof(char *), sizeof(char *), NUM_INIT_ENV_VARS, 1.7f, 0.75f,
                                       str_equals, clean_up, true);

    get_home_dir(env);

    char *bin_path = resolve_variables(env, CONF_VAR_MYIMDB_BIN_DEFAULT_VALUE);
    register_path_dir(env, CONF_VAR_MYIMDB_BIN, bin_path);

    char *etc_path = resolve_variables(env, CONF_VAR_MYIMDB_ETC_DEFAULT_VALUE);
    register_path_dir(env, CONF_VAR_MYIMDB_ETC,  etc_path);

    char *swap_path = resolve_variables(env, CONF_VAR_MYIMDB_SWAP_DEFAULT_VALUE);
    register_path_dir(env, CONF_VAR_MYIMDB_SWAP, swap_path);


    /* change the paths by the optional config-file in home directory */
    char *soft_link_file = resolve_variables(env, CONF_FILE_PATH_SOFTLINKS);
    if ((access(soft_link_file, F_OK ) != -1)) {
        pref_t soft_link_config;
        pref_load(&soft_link_config, soft_link_file);

        update_dir(env, &soft_link_config, CONF_VAR_MYIMDB_BIN, "bin_path", bin_path);
        update_dir(env, &soft_link_config, CONF_VAR_MYIMDB_ETC, "etc_path", etc_path);
        update_dir(env, &soft_link_config, CONF_VAR_MYIMDB_SWAP, "swap_path", swap_path);

        warn("Call to 'pref_free' cannot be execute: NOT IMPLEMENTED (%s)", to_string(conf_load));
        // pref_free(&soft_link_config);    // TODO: Uncomment, when hash table free is implemented
    }

    free (bin_path);
    free (etc_path);
    free (swap_path);

    printf("%s: %s\n", CONF_VAR_MYIMDB_HOME, get_by_key(env, CONF_VAR_MYIMDB_HOME));
    printf("%s: %s\n", CONF_VAR_MYIMDB_BIN,  get_by_key(env, CONF_VAR_MYIMDB_BIN));
    printf("%s: %s\n", CONF_VAR_MYIMDB_ETC,  get_by_key(env, CONF_VAR_MYIMDB_ETC));
    printf("%s: %s\n", CONF_VAR_MYIMDB_SWAP,  get_by_key(env, CONF_VAR_MYIMDB_SWAP));


  //  printf("working dir: %s\n", dir);
    abort();
}

void
conf_get(
    void *out,
    const char *module,
    const char *settings_key)
{

}

void
conf_free()
{

}