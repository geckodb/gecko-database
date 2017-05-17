#include <error.h>
#include <require.h>
#include <containers/vector.h>

struct vector
{
    void *data;
    size_t size, num_elements, capacity;
    enum vector_flags flags;
    float grow_factor;
};

static bool check_vector_create_args(size_t, enum vector_flags, float);
static struct vector *malloc_vector();
static struct vector * malloc_vector_data(struct vector *, enum vector_flags, size_t, size_t);
static void initialize_vector_member(struct vector *, enum vector_flags, size_t, size_t, float);
static bool check_vector_push_back_args(struct vector *, size_t, const void *);
static bool check_vector_auto_resize(struct vector *, size_t);
static bool check_vector_non_null(const struct vector *);
static bool check_vector_set_args(struct vector *, size_t, const void *);
static bool check_set_outside_bounds_enabled(struct vector *, size_t, size_t);
static bool vector_realloc(struct vector *, size_t);
static bool advance(struct vector *, size_t, size_t);

struct vector *vector_create(size_t element_size, size_t capacity)
{
    return vector_create_ex(element_size, capacity, VF_AUTO_RESIZE, GROW_FACTOR);
}

struct vector *vector_create_ex(size_t element_size, size_t capacity, enum vector_flags flags, float grow_factor)
{
    struct vector *result = NULL;
    if (check_vector_create_args(element_size, flags, grow_factor)) {
        result = malloc_vector();
        initialize_vector_member(result, flags, capacity, element_size, grow_factor);
        result = malloc_vector_data(result, flags, capacity, element_size);
    }
    return result;
}

bool vector_push_back(struct vector *vec, size_t num_elements, const void *data)
{
    if (check_vector_push_back_args(vec, num_elements, data) && check_vector_auto_resize(vec, num_elements)) {
        size_t new_num_elements = vec->num_elements + num_elements;
        if (!vector_realloc(vec, new_num_elements))
            return false;
        void *base = vec->data + vec->num_elements * vec->size;
        memcpy(base, data, num_elements * vec->size);
        vec->num_elements = new_num_elements;
        return true;
    } else return false;
}

const void *vector_get_data(struct vector *vec)
{
    return check_vector_non_null(vec) ? vec->data : NULL;
}

bool vector_set(struct vector *vec, size_t idx, size_t num_elements, const void *data)
{
    if (check_vector_non_null(vec) && check_vector_set_args(vec, num_elements, data)) {
        size_t last_idx = idx + num_elements;
        if (last_idx < vec->capacity) {
            memcpy(vec->data + idx * vec->size, data, num_elements * vec->size);
            vec->num_elements = MAX(vec->num_elements, last_idx);
            return true;
        } else if (advance(vec, idx, num_elements))
            return vector_set(vec, idx, num_elements, data);
    }
    return false;
}

size_t vector_get_num_elements(struct vector *vec)
{
    return check_vector_non_null(vec) ? vec->num_elements : 0;
}

size_t vector_get_elements_size(struct vector *vec)
{
    return check_vector_non_null(vec) ? vec->size : 0;
}

bool vector_delete(struct vector *vec)
{
    bool non_null = require_non_null(vec);
    if (non_null) {
        free (vec->data);
    }
    return non_null;
}

bool vector_foreach(struct vector *vec, bool (*consumer)(void *begin, void *end))
{
    if (!check_vector_non_null(vec) || !require_non_null(vec))
        return false;
    return consumer(vec->data, vec->data + vec->num_elements * vec->size);
}

bool vector_equals(const struct vector *lhs, const struct vector *rhs, bool (*comp)(const void *a, const void *b))
{
    if (!check_vector_non_null(lhs) || !check_vector_non_null(rhs))
        return false;
    if ((lhs->num_elements == rhs->num_elements) && (lhs->size == rhs->size)) {
        size_t step = lhs->size;
        size_t end = lhs->num_elements * step;
        for (void *lp = lhs->data; lp != lhs->data + end; lp += step) {
            bool has_found = false;
            for (void *rp = rhs->data; rp != rhs->data + end; rp += step) {
                if (comp(lp, rp)) {
                    has_found = true;
                    goto break_inner_loop;
                }
            }
break_inner_loop:
            if (!has_found) {
                return false;
            }
        }
    }

    return true;
}

bool check_vector_create_args(size_t size, enum vector_flags flags, float grow_factor)
{
    bool result = (size > 0) && (((flags & VF_AUTO_RESIZE) != VF_AUTO_RESIZE) || (grow_factor > 1));
    error_set_last_if(result, EC_ILLEGALARG);
    return result;
}

struct vector *malloc_vector()
{
    struct vector *result = malloc (sizeof(struct vector));
    error_set_last_if((result == NULL), EC_BADMALLOC);
    return result;
}

struct vector * malloc_vector_data(struct vector *vec, enum vector_flags flags, size_t capacity, size_t size)
{
    if (__builtin_expect((vec != NULL) && (vec->data = ((flags & VF_ZERO_MEMORY) == VF_ZERO_MEMORY) ?
                                                       calloc(capacity, size) :
                                                       malloc(capacity * size)) == NULL, false)) {
        error_set_last(EC_BADMALLOC);
        free(vec);
        return NULL;
    } else return vec;
}

void initialize_vector_member(struct vector *vec, enum vector_flags flags, size_t capacity, size_t size, float factor)
{
    if (__builtin_expect(vec != NULL, true)) {
        vec->flags = flags;
        vec->capacity = capacity;
        vec->size = size;
        vec->grow_factor = factor;
        vec->num_elements = 0;
    }
}

bool check_vector_push_back_args(struct vector *vec, size_t num_elements, const void *data)
{
    bool result = ((vec != NULL) && (num_elements > 0) && (data != NULL));
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

bool check_vector_auto_resize(struct vector *vec, size_t num_elements)
{
    bool result = ((vec->size + num_elements < vec->capacity) || (vec->flags & VF_AUTO_RESIZE) == VF_AUTO_RESIZE);
    error_set_last_if(!result, EC_ILLEGALOPP);
    return result;
}

bool check_vector_non_null(const struct vector *vec)
{
    bool result = (vec != NULL);
    error_set_last_if(!result, EC_NULLPOINTER);
    return result;
}

bool check_vector_set_args(struct vector *vec, size_t num_elements, const void *data)
{
    bool result = (vec != NULL && num_elements > 0 && data != NULL);
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

static bool check_set_outside_bounds_enabled(struct vector *vec, size_t idx, size_t num_elements)
{
    bool result = (vec != NULL &&
                   ((idx + num_elements < vec->num_elements) || ((vec->flags & VF_AUTO_RESIZE) == VF_AUTO_RESIZE)));
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

bool vector_realloc(struct vector *vec, size_t new_num_elements)
{
    while (new_num_elements >= vec->capacity)
        vec->capacity *= vec->grow_factor;

    if ((vec->data = realloc(vec->data, vec->capacity * vec->size)) == NULL) {
        error_set_last(EC_BADREALLOC);
        return false;
    } else {
        if (vec->flags & VF_ZERO_MEMORY) {
            void *base = vec->data + vec->num_elements * vec->size;
            memset(base, 0, (vec->capacity - vec->num_elements) * vec->size);
        }
        return true;
    }
}

static bool advance(struct vector *vec, size_t idx, size_t num_elements)
{
    if ((idx + num_elements) < vec->num_elements) {
        return true;
    } else if (check_set_outside_bounds_enabled(vec, idx, num_elements)) {
        if (idx < vec->num_elements)
            return advance(vec, vec->num_elements, num_elements - vec->num_elements);
        else return vector_realloc(vec, idx + num_elements);
    }
    return false;
}