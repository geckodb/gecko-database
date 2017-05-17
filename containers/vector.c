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

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static bool _check_alloc_args(size_t, enum vector_flags, float);
static mdb_vector *_new_vec();
static mdb_vector * _new_vec_data(mdb_vector *, enum vector_flags, size_t, size_t);
static void _init_vec(mdb_vector *, enum vector_flags, size_t, size_t, float);
static bool _check_add_args(mdb_vector *, size_t, const void *);
static bool _check_auto_resize(mdb_vector *, size_t);
static bool check_vector_set_args(mdb_vector *, size_t, const void *);
static bool check_set_outside_bounds_enabled(mdb_vector *, size_t, size_t);
static bool vector_realloc(mdb_vector *, size_t);
static bool advance(mdb_vector *, size_t, size_t);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

mdb_vector *mdb_vector_alloc(size_t element_size, size_t capacity)
{
    return mdb_vector_alloc_ex(element_size, capacity, VF_AUTO_RESIZE, GROW_FACTOR);
}

mdb_vector *mdb_vector_alloc_ex(size_t element_size, size_t capacity, enum vector_flags flags, float grow_factor)
{
    mdb_vector *result = NULL;
    if (_check_alloc_args(element_size, flags, grow_factor)) {
        result = _new_vec();
        _init_vec(result, flags, capacity, element_size, grow_factor);
        result = _new_vec_data(result, flags, capacity, element_size);
    }
    return result;
}

bool mdb_vector_free(mdb_vector *vec)
{
    bool non_null = mdb_require_non_null(vec);
    if (non_null) {
        free (vec->data);
    }
    return non_null;
}

bool mdb_vector_add(mdb_vector *vec, size_t num_elements, const void *data)
{
    if (_check_add_args(vec, num_elements, data) && _check_auto_resize(vec, num_elements)) {
        size_t new_num_elements = vec->num_elements + num_elements;
        if (!vector_realloc(vec, new_num_elements))
            return false;
        void *base = vec->data + vec->num_elements * vec->sizeof_element;
        memcpy(base, data, num_elements * vec->sizeof_element);
        vec->num_elements = new_num_elements;
        return true;
    } else return false;
}

const void *mdb_vector_get(mdb_vector *vec)
{
    return mdb_require_non_null(vec) ? vec->data : NULL;
}

bool mdb_vector_set(mdb_vector *vec, size_t idx, size_t num_elements, const void *data)
{
    if (mdb_require_non_null(vec) && check_vector_set_args(vec, num_elements, data)) {
        size_t last_idx = idx + num_elements;
        if (last_idx < vec->element_capacity) {
            memcpy(vec->data + idx * vec->sizeof_element, data, num_elements * vec->sizeof_element);
            vec->num_elements = MAX(vec->num_elements, last_idx);
            return true;
        } else if (advance(vec, idx, num_elements))
            return mdb_vector_set(vec, idx, num_elements, data);
    }
    return false;
}

size_t vector_get_num_elements(mdb_vector *vec)
{
    return mdb_require_non_null(vec) ? vec->num_elements : 0;
}

size_t vector_get_elements_size(mdb_vector *vec)
{
    return mdb_require_non_null(vec) ? vec->sizeof_element : 0;
}

bool mdb_vector_foreach(mdb_vector *vec, void *capture, bool (*func)(void *capture, void *begin, void *end))
{
    if (!mdb_require_non_null(vec) || !mdb_require_non_null(vec))
        return false;
    return func(capture, vec->data, vec->data + vec->num_elements * vec->sizeof_element);
}

bool mdb_vector_comp(const mdb_vector *lhs, const mdb_vector *rhs, bool (*comp)(const void *a, const void *b))
{
    if (!mdb_require_non_null(lhs) || !mdb_require_non_null(rhs))
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

bool _check_alloc_args(size_t size, enum vector_flags flags, float grow_factor)
{
    bool result = (size > 0) && (((flags & VF_AUTO_RESIZE) != VF_AUTO_RESIZE) || (grow_factor > 1));
    error_set_last_if(result, EC_ILLEGALARG);
    return result;
}

mdb_vector *_new_vec()
{
    mdb_vector *result = malloc (sizeof(mdb_vector));
    error_set_last_if((result == NULL), EC_BADMALLOC);
    return result;
}

mdb_vector * _new_vec_data(mdb_vector *vec, enum vector_flags flags, size_t capacity, size_t size)
{
    if (__builtin_expect((vec != NULL) && (vec->data = ((flags & VF_ZERO_MEMORY) == VF_ZERO_MEMORY) ?
                                                       calloc(capacity, size) :
                                                       malloc(capacity * size)) == NULL, false)) {
        error_set_last(EC_BADMALLOC);
        free(vec);
        return NULL;
    } else return vec;
}

void _init_vec(mdb_vector *vec, enum vector_flags flags, size_t capacity, size_t size, float factor)
{
    if (__builtin_expect(vec != NULL, true)) {
        vec->flags = flags;
        vec->element_capacity = capacity;
        vec->sizeof_element = size;
        vec->grow_factor = factor;
        vec->num_elements = 0;
    }
}

bool _check_add_args(mdb_vector *vec, size_t num_elements, const void *data)
{
    bool result = ((vec != NULL) && (num_elements > 0) && (data != NULL));
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

bool _check_auto_resize(mdb_vector *vec, size_t num_elements)
{
    bool result = ((vec->sizeof_element + num_elements < vec->element_capacity) ||
                   (vec->flags & VF_AUTO_RESIZE) == VF_AUTO_RESIZE);
    error_set_last_if(!result, EC_ILLEGALOPP);
    return result;
}

bool check_vector_set_args(mdb_vector *vec, size_t num_elements, const void *data)
{
    bool result = (vec != NULL && num_elements > 0 && data != NULL);
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

static bool check_set_outside_bounds_enabled(mdb_vector *vec, size_t idx, size_t num_elements)
{
    bool result = (vec != NULL &&
                   ((idx + num_elements < vec->num_elements) || ((vec->flags & VF_AUTO_RESIZE) == VF_AUTO_RESIZE)));
    error_set_last_if(!result, EC_ILLEGALARG);
    return result;
}

bool vector_realloc(mdb_vector *vec, size_t new_num_elements)
{
    while (new_num_elements >= vec->element_capacity)
        vec->element_capacity *= vec->grow_factor;

    if ((vec->data = realloc(vec->data, vec->element_capacity * vec->sizeof_element)) == NULL) {
        error_set_last(EC_BADREALLOC);
        return false;
    } else {
        if (vec->flags & VF_ZERO_MEMORY) {
            void *base = vec->data + vec->num_elements * vec->sizeof_element;
            memset(base, 0, (vec->element_capacity - vec->num_elements) * vec->sizeof_element);
        }
        return true;
    }
}

static bool advance(mdb_vector *vec, size_t idx, size_t num_elements)
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