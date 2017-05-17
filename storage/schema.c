#include <error.h>
#include <storage/schema.h>
#include <containers/vector.h>
#include <require.h>


static bool check_valid_schema(struct schema *);
static bool check_attribute_idx_in_bounds(struct schema *, size_t);
static bool free_attribute_ptr(void *, void *);

struct schema *schema_create(size_t attribute_capacity)
{
    struct schema *result;
    if ((result = malloc (sizeof(struct schema))) == NULL) {
        error_set_last(EC_BADMALLOC);
    } else {
        result->attributes = vector_create(sizeof(attribute_get_sizeof_ptr()), attribute_capacity);
    }
    return result;
}

bool schema_delete(struct schema *s)
{
    bool result = check_valid_schema(s);
    if (result) {
        if (s->attributes != NULL) {
            vector_foreach(s->attributes, free_attribute_ptr);
            vector_delete(s->attributes);
            s->attributes = NULL;
        }
        free (s);
    }
    return result;
}

bool schema_equals(struct schema *lhs, struct schema *rhs)
{
    return (check_valid_schema(lhs) && check_valid_schema(rhs) &&
            vector_equals(lhs->attributes, rhs->attributes, attribute_equals));
}

size_t schema_get_num_of_attributes(struct schema *s)
{
    return (check_valid_schema(s) ? vector_get_num_elements(s->attributes) : 0);
}

bool schema_set_attribute(struct schema *s, size_t attribute_idx, const struct attribute *attr)
{
    bool result = (check_valid_schema(s) && check_attribute_non_null(attr));
    if (result) {
        struct attribute *cpy = attribute_copy(attr);
        vector_set(s->attributes, attribute_idx, 1, &cpy);
    }
    return result;
}

enum data_type schema_get_attribute_type(struct schema *s, size_t attribute_idx)
{
    if (check_valid_schema(s) && check_attribute_idx_in_bounds(s, attribute_idx)) {
        return attribute_get_type((const struct attribute *) vector_get_data(s->attributes));
    } else return DT_UNDEFINED;
}

const char *schema_get_attribute_name(struct schema *s, size_t attribute_idx)
{
    if (check_valid_schema(s) && check_attribute_idx_in_bounds(s, attribute_idx)) {
        return attribute_get_name((const struct attribute *) vector_get_data(s->attributes));
    } else return NULL;
}

bool check_valid_schema(struct schema *s)
{
    bool result = ((s != NULL) && (s->attributes != NULL));
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

bool check_attribute_idx_in_bounds(struct schema *s, size_t attribute_idx)
{
    bool result = (attribute_idx < vector_get_num_elements(s->attributes));
    error_set_last_if(!result, EC_OUTOFBOUNDS);
    return result;
}

bool free_attribute_ptr(void *begin, void *end)
{
    bool result = require_non_null(begin) && require_non_null(end) && require_less_than(begin, end);
    for (struct attribute **it = (struct attribute **) begin; it < (struct attribute **) end; it++)
        result &= attribute_delete(*it);
    return result;
}