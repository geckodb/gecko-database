// An implementation of the vector data structure
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

#include <error.h>
#include <require.h>
#include <containers/vector.h>
#include <math.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static bool      _check_create_args(size_t, vector_flags, float);
static vector_t *_alloc_vector();
static vector_t *_alloc_data(vector_t *, vector_flags, size_t, size_t);
static void      _init_vector(vector_t *, vector_flags, size_t, size_t, float);
static bool      _check_add_args(vector_t *, size_t, const void *);
static bool      _check_auto_resize(vector_t *, size_t);
static bool      _check_set_args(vector_t *, size_t, const void *);
static bool      _outside_bounds_enabled(vector_t *, size_t, size_t);
static bool      _realloc_vector(vector_t *, size_t);
static bool      _advance(vector_t *, size_t, size_t);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

vector_t *vector_create(size_t element_size, size_t capacity)
{
    return vector_create_ex(element_size, capacity, auto_resize, GROW_FACTOR);
}

vector_t *vector_create_ex(size_t element_size, size_t capacity, vector_flags flags, float grow_factor)
{
    vector_t *result = NULL;
    if (_check_create_args(element_size, flags, grow_factor)) {
        result = _alloc_vector();
        _init_vector(result, flags, capacity, element_size, grow_factor);
        result = _alloc_data(result, flags, capacity, element_size);
    }
    return result;
}

bool vector_resize(vector_t *vec, size_t num_elements)
{
    if (require_non_null(vec) && require_non_zero(num_elements) && _advance(vec, 0, num_elements)) {
        vec->num_elements = num_elements;
        return true;
    } else return false;
}

vector_t *vector_cpy(vector_t *proto)
{
    vector_t *result = NULL;
    if ((require_non_null(proto)) && (require_non_null(proto->data)) &&
        ((result = vector_create_ex(proto->sizeof_element, proto->element_capacity, proto->flags, proto->grow_factor)))) {
        vector_set(result, 0, proto->num_elements, proto->data);
    }
    return result;
}

bool vector_free(vector_t *vec)
{
    bool non_null = require_non_null(vec);
    if (non_null) {
        free (vec->data);
    }
    return non_null;
}

bool vector_free_ex(vector_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end))
{
    return vector_foreach(vec, capture, func) && vector_free(vec);
}

bool vector_add(vector_t *vec, size_t num_elements, const void *data)
{
    if (_check_add_args(vec, num_elements, data) && _check_auto_resize(vec, num_elements)) {
        size_t new_num_elements = vec->num_elements + num_elements;
        if (!_realloc_vector(vec, new_num_elements))
            return false;
        void *base = vec->data + vec->num_elements * vec->sizeof_element;
        memcpy(base, data, num_elements * vec->sizeof_element);
        vec->num_elements = new_num_elements;
        return true;
    } else return false;
}

void *vector_get(vector_t *vec)
{
    return require_non_null(vec) ? vec->data : NULL;
}

void *vector_at(const vector_t *vec, size_t pos)
{
    void *result = NULL;
    if (require_non_null(vec) && require_constraint_size_t(pos, constraint_less, vec->num_elements))
       result = (vec->data + pos * vec->sizeof_element);
    return result;
}

void *vector_peek(const vector_t *vec)
{
    void *result = NULL;
    if (require_non_null(vec) && require_non_zero(vec->num_elements))
        result = vector_at(vec, vec->num_elements - 1);
    return result;
}

bool vector_set(vector_t *vec, size_t idx, size_t num_elements, const void *data)
{
    if (require_non_null(vec) && _check_set_args(vec, num_elements, data)) {
        size_t last_idx = idx + num_elements;
        if (last_idx < vec->element_capacity) {
            memcpy(vec->data + idx * vec->sizeof_element, data, num_elements * vec->sizeof_element);
            vec->num_elements = max(vec->num_elements, last_idx);
            return true;
        } else if (_advance(vec, idx, num_elements))
            return vector_set(vec, idx, num_elements, data);
    }
    return false;
}

size_t vector_get_num_elements(vector_t *vec)
{
    return require_non_null(vec) ? vec->num_elements : 0;
}

size_t vector_get_elements_size(vector_t *vec)
{
    return require_non_null(vec) ? vec->sizeof_element : 0;
}

bool vector_foreach(vector_t *vec, void *capture, bool (*func)(void *capture, void *begin, void *end))
{
    if (!require_non_null(vec) || !require_non_null(func))
        return false;
    return func(capture, vec->data, vec->data + vec->num_elements * vec->sizeof_element);
}

size_t vector_count(vector_t *vec, void *capture, bool (*pred)(void *capture, void *it))
{
    size_t count = 0;
    if (require_non_null(vec) && require_non_null(pred)) {
        void *end = (vec->data + vec->num_elements * vec->sizeof_element);
        for (void *it = vec->data; it != end; it += vec->sizeof_element) {
            count += pred(capture, it);
        }
    }
    return count;
}

bool vector_contains(vector_t *vec, void *needle)
{
    if (require_non_null(vec) && require_non_null(needle)) {
        void *end = (vec->data + vec->num_elements * vec->sizeof_element);
        for (void *chunk_start = vec->data; chunk_start < end; chunk_start += vec->sizeof_element) {
            void *chunk_end = (chunk_start + vec->sizeof_element);
            bool is_equal = true;
            for (void *byte = chunk_start; byte < chunk_end; byte++) {
                size_t byte_num = (byte - chunk_start);
                if (*(char *) byte != *((char *) needle + byte_num)) {
                    is_equal = false;
                    break;
                }
            }
            if (is_equal)
                return true;
        }
    }
    return false;
}

bool vector_comp(const vector_t *lhs, const vector_t *rhs, bool (*comp)(const void *a, const void *b))
{
    if (!require_non_null(lhs) || !require_non_null(rhs))
        return false;
    if ((lhs->num_elements == rhs->num_elements) && (lhs->sizeof_element == rhs->sizeof_element)) {
        size_t step = lhs->sizeof_element;
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

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool _check_create_args(size_t size, vector_flags flags, float grow_factor)
{
    bool result = (size > 0) && (((flags & auto_resize) != auto_resize) || (grow_factor > 1));
    error_if(result, err_illegal_args);
    return result;
}

vector_t *_alloc_vector()
{
    vector_t *result = malloc (sizeof(vector_t));
    error_if((result == NULL), err_bad_malloc);
    return result;
}

vector_t *_alloc_data(vector_t *vec, vector_flags flags, size_t capacity, size_t size)
{
    if (__builtin_expect((vec != NULL) && (vec->data = ((flags & zero_memory) == zero_memory) ?
                                                       calloc(capacity, size) :
                                                       malloc(capacity * size)) == NULL, false)) {
        error(err_bad_malloc);
        free(vec);
        return NULL;
    } else return vec;
}

void _init_vector(vector_t *vec, vector_flags flags, size_t capacity, size_t size, float factor)
{
    if (__builtin_expect(vec != NULL, true)) {
        vec->flags = flags;
        vec->element_capacity = capacity;
        vec->sizeof_element = size;
        vec->grow_factor = factor;
        vec->num_elements = 0;
    }
}

bool _check_add_args(vector_t *vec, size_t num_elements, const void *data)
{
    bool result = ((vec != NULL) && (num_elements > 0) && (data != NULL));
    error_if(!result, err_illegal_args);
    return result;
}

bool _check_auto_resize(vector_t *vec, size_t num_elements)
{
    bool result = ((vec->sizeof_element + num_elements < vec->element_capacity) ||
                   (vec->flags & auto_resize) == auto_resize);
    error_if(!result, err_illegal_opp);
    return result;
}

bool _check_set_args(vector_t *vec, size_t num_elements, const void *data)
{
    bool result = (vec != NULL && num_elements > 0 && data != NULL);
    error_if(!result, err_illegal_args);
    return result;
}

static bool _outside_bounds_enabled(vector_t *vec, size_t idx, size_t num_elements)
{
    bool result = (vec != NULL &&
                   ((idx + num_elements < vec->num_elements) || ((vec->flags & auto_resize) == auto_resize)));
    error_if(!result, err_illegal_args);
    return result;
}

bool _realloc_vector(vector_t *vec, size_t new_num_elements)
{
    while (new_num_elements >= vec->element_capacity)
        vec->element_capacity = ceil(vec->element_capacity * vec->grow_factor);

    if ((vec->data = realloc(vec->data, vec->element_capacity * vec->sizeof_element)) == NULL) {
        error(err_bad_realloc);
        return false;
    } else {
        if ((vec->flags & zero_memory) == zero_memory) {
            void *base = vec->data + vec->num_elements * vec->sizeof_element;
            memset(base, 0, (vec->element_capacity - vec->num_elements) * vec->sizeof_element);
        }
        return true;
    }
}

static bool _advance(vector_t *vec, size_t idx, size_t num_elements)
{
    if ((idx + num_elements) < vec->num_elements) {
        return true;
    } else if (_outside_bounds_enabled(vec, idx, num_elements)) {
        if (idx < vec->num_elements)
            return _advance(vec, vec->num_elements, num_elements - vec->num_elements);
        else return _realloc_vector(vec, idx + num_elements);
    }
    return false;
}