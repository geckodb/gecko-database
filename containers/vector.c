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

#include <containers/vector.h>
#include <containers/dicts/hash_table.h>

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
static bool inline _advance(vector_t *, size_t, size_t);

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

bool vector_reserve(vector_t *vec, size_t num_elements)
{
    return _advance(vec, 0, num_elements - 1);
}

size_t vector_num_elements(const vector_t *vec)
{
    require_non_null(vec);
    return (vec->num_elements);
}

void vector_memset(vector_t *vec, size_t pos_start, size_t num_elements, const void *data)
{
    require_nonnull(vec);
    require((num_elements > 0), "illegal argument");
    require((pos_start + num_elements <= vec->element_capacity), "out of bounds");
    for (size_t i = pos_start; i < pos_start + num_elements; i++) {
        vector_set(vec, i, 1, data);
    }
    vec->is_sorted = false;
}

vector_t *vector_deep_cpy(vector_t *proto)
{
    vector_t *result = NULL;
    if ((require_non_null(proto)) && (require_non_null(proto->data)) &&
        ((result = vector_create_ex(proto->sizeof_element, proto->element_capacity, proto->flags, proto->grow_factor)))) {
        vector_set(result, 0, proto->num_elements, proto->data);
    }
    return result;
}

bool vector_free(struct vector_t *vec)
{
    bool non_null = require_non_null(vec);
    if (non_null) {
        if (require_non_null(vec->data)) {
            free (vec->data);
        }
        free (vec);
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
        vec->is_sorted = false;
        return true;
    } else return false;
}

bool vector_add_all(vector_t *dest, const vector_t *src)
{
    if (dest == NULL || src == NULL || dest->sizeof_element != src->sizeof_element) {
        return false;
    } else {
        vector_reserve(dest, dest->num_elements + src->num_elements);
        return vector_add_all_unsafe(dest, src);
    }
}

bool vector_add_all_unsafe(vector_t *dest, const vector_t *src)
{
    if (dest == NULL || src == NULL || dest->sizeof_element != src->sizeof_element) {
        return false;
    } else {
        vector_add_unsafe(dest, src->num_elements, src->data);
        return true;
    }
}

void vector_add_unsafe(vector_t *vec, size_t num_elements, const void *data)
{
    size_t new_num_elements = vec->num_elements + num_elements;
    void *base = vec->data + vec->num_elements * vec->sizeof_element;
    memcpy(base, data, num_elements * vec->sizeof_element);
    vec->num_elements = new_num_elements;
    vec->is_sorted = false;
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

void *vector_begin(const vector_t *vec)
{
    void *result = NULL;
    if (require_non_null(vec) && require_non_zero(vec->num_elements))
        result = vector_at(vec, 0);
    return result;
}

void *vector_end(const vector_t *vec)
{
    void *result = NULL;
    if (require_non_null(vec))
        result = vector_at(vec, vec->num_elements);
    return (result == NULL ? (vec->data + vec->num_elements * vec->sizeof_element) : result);
}

bool vector_issorted(vector_t *vec, cache_consideration_policy policy, comp_t comp)
{
    require_non_null(vec);
    require((policy != CCP_IGNORECACHE || (comp != NULL)), "Null pointer to required function pointer");
    return (policy == CCP_USECACHE) ? vec->is_sorted : vector_updatesort(vec, comp);
}

bool vector_updatesort(vector_t *vec, comp_t comp)
{
    require_non_null(vec);
    require_non_null(comp);

    // update sort state
    vec->is_sorted = true;
    size_t num = vec->num_elements - 1;
    void *lhs = vec->data, *rhs = vec->data + vec->sizeof_element;
    while (num--) {
        vec->is_sorted &= (comp(lhs, rhs) < 1);
        lhs += vec->sizeof_element;
        rhs += vec->sizeof_element;
    }
    return vec->is_sorted;
}

void *vector_pop_unsafe(vector_t *vec)
{
    return (vec->data + (vec->num_elements-- - 1) * vec->sizeof_element);
}

void *vector_peek_unsafe(vector_t *vec)
{
    return (vec->data + (vec->num_elements - 1) * vec->sizeof_element);
}

bool vector_set(vector_t *vec, size_t idx, size_t num_elements, const void *data)
{
    if (require_non_null(vec) && _check_set_args(vec, num_elements, data)) {
        vec->is_sorted = false;
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
    vec->is_sorted = false;
    return func(capture, vec->data, vec->data + vec->num_elements * vec->sizeof_element);
}

void vector_dedup(vector_t *vec)
{
    dict_t *dict = hash_table_create_jenkins(vec->sizeof_element, sizeof(bool), vec->num_elements, 2.0f, 0.75f);
    void *end = vector_end(vec);
    bool dummy;
    for (void *it = vector_begin(vec); it < end; dict_put(dict, it++, &dummy));
    vector_t *dedups = (vector_t *) dict_keyset(dict);
    vector_swap(vec, dedups);
    dict_free(dict);
    vector_free(dedups);
}

void vector_shallow_cpy(vector_t *dst, vector_t *src)
{
    dst->sizeof_element = src->sizeof_element;
    dst->num_elements = src->num_elements;
    dst->element_capacity = src->element_capacity;
    dst->flags = src->flags;
    dst->grow_factor = src->grow_factor;
    dst->data = src->data;
    dst->is_sorted = src->is_sorted;
}

void vector_swap(vector_t *lhs, vector_t *rhs)
{
    vector_t tmp;
    vector_shallow_cpy(&tmp, lhs);
    vector_shallow_cpy(lhs, rhs);
    vector_shallow_cpy(rhs, &tmp);
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

size_t vector_memused(vector_t *vec)
{
    return (vec->element_capacity * vec->sizeof_element);
}

size_t vector_memused__str(vector_t *vec)
{
    size_t memused = vector_memused(vec);
    size_t total_str_size = 0;
    vector_foreach(vec, &total_str_size, get_sizeof_strings);
    return (memused + total_str_size);
}

size_t vector_sizeof(const vector_t *vec)
{
    return (vec == NULL ? 0 : vec->element_capacity * vec->sizeof_element);
}

void vector_sort(vector_t *vec, comp_t comp)
{
    require_non_null(vec);
    require_non_null(comp);

    if (!vec->is_sorted) {
        mergesort(vec->data, vec->num_elements, vec->sizeof_element, comp);
        vec->is_sorted = true;
    }
}

void *vector_bsearch(vector_t *vec, const void *needle, comp_t sort_comp, comp_t find_comp)
{
    require_non_null(vec);
    require_non_null(needle);
    require_non_null(sort_comp);
    require_non_null(find_comp);

    vector_sort(vec, sort_comp);

    size_t lower = 0;
    size_t upper = vec->num_elements;
    size_t mid;

    while(lower <= upper) {
        mid = lower + (upper - lower) / 2;
        void *element = vec->data + mid * vec->sizeof_element;
        int comp = find_comp(needle, element);
        if(comp == 0) {
            return element;
        } else {
            if(comp < 0)
                lower = mid + 1;
            else
                upper = mid -1;
        }
    }

    return vector_end(vec);
}

bool vector_comp(const vector_t *lhs, const vector_t *rhs, comp_t comp)
{
    if (!require_non_null(lhs) || !require_non_null(rhs))
        return false;
    if ((lhs->num_elements == rhs->num_elements) && (lhs->sizeof_element == rhs->sizeof_element)) {
        size_t step = lhs->sizeof_element;
        size_t end = lhs->num_elements * step;
        for (void *lp = lhs->data; lp != lhs->data + end; lp += step) {
            bool has_found = false;
            for (void *rp = rhs->data; rp != rhs->data + end; rp += step) {
                if (comp(lp, rp) == 0) {
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


bool
vector_free__str(
    vector_t *vec)
{
    return vector_foreach(vec, NULL, free_strings) && vector_free(vec);
}

bool
free_strings(
        void *capture,
        void *begin,
        void *end)
{
    bool result = require_non_null(begin) && require_non_null(end) && require_less_than(begin, end);
    for (char **it = (char **) begin; it < (char **) end; it++) {
        free (*it);
    }

    return result;
};

bool
get_sizeof_strings(
        void *capture,
        void *begin,
        void *end)
{
    size_t total_size = 0;
    bool result = require_non_null(begin) && require_non_null(end) && require_less_than(begin, end);
    for (char **it = (char **) begin; it < (char **) end; it++) {
        total_size += (strlen (*it) + 1);
    }
    *((size_t *) capture) = total_size;
    return result;
};



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
        vec->is_sorted = true;
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
    if (new_num_elements >= vec->element_capacity) {
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
    } else return true;
}

static bool inline  _advance(vector_t *vec, size_t idx, size_t num_elements)
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