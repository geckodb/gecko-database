#include <pref.h>
#include <error.h>
#include <require.h>
#include <unistd.h>
#include <containers/dictionaries/fixed_linear_hash_table.h>
#include <ctype.h>

#define NOSUCHFILE          "unable to load preference file '%s' (file not found)"

vector_t *
read_lines(
        const char *file
);

typedef enum {
        TOKEN_COMMENT,
        TOKEN_KEY,
        TOKEN_ASSIGNMENT,
        TOKEN_VALUE,
        TOKEN_UNKNOWN
} token_t;

void
pref_load(
        pref_t *pref,
        const char *file)
{
    require_non_null(pref);
    require_non_null(file);

    panic_if((access(file, F_OK ) == -1), NOSUCHFILE, file);

    pref->last_read = time(NULL);
    pref->dict = hash_table_create_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                   sizeof(char *), sizeof(char *), NUM_INIT_CONFIG_STATEMENTS, 1.7f, 0.75f,
                                   str_equals, clean_up, true);

    vector_t *lines = read_lines(file);

    for (int i = 0; i < lines->num_elements; i++) {
        const char *line = *(const char **) vector_at(lines, i);
        token_t token = TOKEN_UNKNOWN;

        const char *key_begin = NULL, *key_end, *value_begin = NULL, *value_end = NULL;

        for (const char *symbol = line; *symbol; symbol++) {
            const char character = symbol[0];
            switch (token) {
                case TOKEN_UNKNOWN:
                    if (isspace(character))
                        continue;
                    else if (character == '#') {
                        token = TOKEN_COMMENT;
                    } else {
                        key_begin = symbol;
                        token = TOKEN_KEY;
                    }
                    break;
                case TOKEN_COMMENT:
                    goto next_line;
                case TOKEN_KEY:
                    if (isspace(character)) {
                        token = TOKEN_ASSIGNMENT;
                        key_end = symbol;
                    }
                    break;
                case TOKEN_ASSIGNMENT:
                    if (isspace(character))
                        continue;
                    else {
                        value_begin = symbol;
                        token = TOKEN_VALUE;
                    }
                    break;
                case TOKEN_VALUE:
                    value_end = symbol;
                    break;
            }
        }
next_line:

        if (value_end != NULL) {
            value_end = value_begin + (value_end - value_begin);
            while(value_end > value_begin && isspace((unsigned char)*value_end))
                value_end--;

            size_t key_size = key_end - key_begin;
            size_t value_size = value_end - value_begin + 1;
            char *import_key   = malloc(key_size   + 1);
            char *import_value = malloc(value_size + 1);
            memcpy(import_key, key_begin, key_size);
            memcpy(import_value, value_begin, value_size);
            import_key[key_size] = import_value[value_size] = '\0';

            dict_put(pref->dict, &import_key, &import_value);
        }
    }

    vector_foreach(lines, NULL, free_strings);
    vector_free(lines);
}

void
pref_free(
    pref_t *pref)
{
    if (pref) {
        if (pref->dict)
            dict_free(pref->dict);
        free (pref);
    }
}

bool
pref_has(
    const pref_t *pref,
    const char *key)
{
    return (pref_get_str(pref, key, NULL) != NULL);
}

const char *pref_get_str(
    const pref_t *pref,
    const char *key,
    const char *default_val)
{
    require_non_null(pref);
    require_non_null(pref->dict);
    require_non_null(key);

    const char **value = (const char **) dict_get(pref->dict, &key);
    return (value != NULL ? (*value) : default_val);
}

void pref_get_uint32(
    unsigned *out,
    const pref_t *pref,
    const char *key,
    unsigned *default_val)
{
    const char *value = pref_get_str(pref, key, NULL);

    if (value == NULL) {
        *out = *default_val;
    } else {
        for (const char *it = value ; *it ; it++) {
            if (!isdigit(*it)) {
                *out = *default_val;
                return;
            }
        }
    }

    *out = (strtoul(value, 0L, 10));
}

vector_t *
read_lines(
    const char *file)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    vector_t *result = vector_create(sizeof(char *), 100);

    fp = fopen(file, "r");
    panic_if((fp == NULL), NOSUCHFILE, file);

    while ((read = getline(&line, &len, fp)) != -1) {
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0'; // removing the newline character
        }

        const char *dup = strdup(line);
        vector_add(result, 1, &dup);
    }

    fclose(fp);
    if (line)
        free(line);

    return result;
}
