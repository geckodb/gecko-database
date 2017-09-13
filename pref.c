// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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

#include <pref.h>
#include <containers/dicts/hash_table.h>

#define NOSUCHFILE          "unable to load configuration file '%s' (file not found)"

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
        const char *file,
        dict_t *dict,
        char * (*resolve_variables)(dict_t *dict, const char *string))
{
    REQUIRE_NONNULL(pref)
    REQUIRE_NONNULL(file)

   // panic_if((access(file, F_OK ) == -1), NOSUCHFILE, file); // TODO: Hack

    pref->last_read = time(NULL);
    pref->dict = hash_table_create_ex(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                   sizeof(char *), sizeof(char *), NUM_INIT_CONFIG_STATEMENTS,
                                   NUM_INIT_CONFIG_STATEMENTS, 1.7f, 0.75f,
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

            if (resolve_variables != NULL && dict != NULL) {
                import_value = resolve_variables(dict, import_value);
            }

            dict_put(pref->dict, &import_key, &import_value);
        }
    }

    vector_free__str(lines);
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
    REQUIRE_NONNULL(pref);
    REQUIRE_NONNULL(pref->dict);
    REQUIRE_NONNULL(key);

    const char **value = (const char **) dict_get(pref->dict, &key);
    return (value != NULL ? (*value) : default_val);
}

void pref_get_size_t(
    size_t *out,
    const pref_t *pref,
    const char *key,
    size_t *default_val)
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
