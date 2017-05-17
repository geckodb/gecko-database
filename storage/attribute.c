#include <error.h>
#include <storage/attribute.h>
#include <require.h>

static bool check_attribute_create_args(const char *name);
static size_t get_sizeof_type(enum data_type type);

struct attribute
{
    enum data_type type;
    size_t type_size;
    const char *name;
};

struct attribute *attribute_create(enum data_type type, const char *name)
{
    struct attribute *result = NULL;
    if (check_attribute_create_args(name)) {
        if ((result = malloc (sizeof(result))) == NULL) {
            error_set_last(EC_BADMALLOC);
        } else {
            result->name = strdup(name);
            result->type_size = get_sizeof_type(type);
            result->type = type;
        }
    }
    return result;
}

bool attribute_delete(struct attribute *attr)
{
    bool non_null = check_attribute_non_null(attr);
    if (non_null) {
        free ((void *) attr->name);
        free (attr);
    }
    return non_null;
}

enum data_type attribute_get_type(const struct attribute *attr)
{
    return (check_attribute_non_null(attr) ? attr->type : DT_UNDEFINED);
}

const char *attribute_get_name(const struct attribute *attr)
{
    return (check_attribute_non_null(attr) ? attr->name : NULL);
}

bool attribute_equals(const void *a, const void *b)
{
    const struct attribute *lhs = (const struct attribute *) a;
    const struct attribute *rhs = (const struct attribute *) b;
    return (check_attribute_non_null(lhs) && check_attribute_non_null(rhs)) &&
           ((lhs->type == rhs->type) && (strcmp(lhs->name, rhs->name) == 0));
}

bool check_attribute_create_args(const char *name)
{
    bool result;
    if ((result = (name != NULL && strlen(name) > 0)) == false)
        error_set_last(EC_ILLEGALARG);
    return result;
}

size_t get_sizeof_type(enum data_type type)
{
    switch (type) {
        case DT_BOOLEAN: case DT_UNSIGNED_BYTE: case DT_SIGNED_BYTE: return 1;
        case DT_UNSIGNED_SHORT: case DT_SIGNED_SHORT: return 2;
        case DT_UNSIGNED_INT: case DT_SIGNED_INT: case DT_FLOAT: case DT_STRING: return 4;
        case DT_UNSIGNED_LONG: case DT_SIGNED_LONG: case DT_DOUBLE: case DT_INTERNAL_TID: return 8;
        default: {
            error_set_last(EC_UNKNOWNTYPE);
            return 0;
        }
    }
}

bool check_attribute_non_null(const struct attribute *attr)
{
    bool result = (attr != NULL);
    if (!result) {
        error_set_last(EC_NULLPOINTER);
    }
    return result;
}

struct attribute *attribute_copy(const struct attribute *attr)
{
    if (attr != NULL) {
        struct attribute *cpy = malloc (sizeof(struct attribute));
        cpy->name = strdup(attr->name);
        cpy->type = attr->type;
        cpy->type_size = attr->type_size;
        return cpy;
    } else return NULL;
}

size_t attribute_get_sizeof_ptr()
{
    return sizeof(struct attribute *);
}

