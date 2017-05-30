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

#include <storage/memory.h>
#include <error.h>
#include <assert.h>
#include <limits.h>
#include <containers/vector.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define NULL_FRAME_ID   UINT_MAX
#define MAX_OFFSET      SIZE_MAX
#define NULL_OFFSET     0
#define MIN_OFFSET      0
#define CORE_HDR_SIZE   (sizeof(page_header_t) + sizeof(free_space_register_header_t) + sizeof(frame_register_header_t))
#define FRAME_HDR_SIZE  (sizeof(vframe_t))
#define MINPAYLOAD_SIZE (sizeof(zone_header_t) + sizeof(zone_ptr))

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    target_free_space_reg,
    target_frame_reg,
    target_free_space_data,
    target_frame_reg_inuse_data,
    target_frame_reg_freelist_data,
    target_free_space_begin
} seek_target;

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define output_error(code)                                                                                             \
    {                                                                                                                  \
        error(code);                                                                                                   \
        error_print();                                                                                                 \
        fprintf(stderr, "\t> occurred here: '%s:%d'\n", __FILE__, __LINE__);                                           \
        fflush(stderr);                                                                                                \
        exit(1);                                                                                                       \
    }

#define return_if(expr, error_code, retval)                                                                            \
    if (expr) {                                                                                                        \
        output_error(error_code);                                                                                      \
        return retval;                                                                                                 \
    }

#define warn_if(expr, msg, args...)                                                                                    \
    if (expr) {                                                                                                        \
        fprintf(stderr, "WARNING (%s:%d): ", __FILE__, __LINE__);                                                      \
        fprintf(stderr, msg, args);                                                                                    \
        fflush(stderr);                                                                                                \
    }

#define debug_var(type, name, value)                                                                                   \
    type name = value;


#define expect_non_null(obj, retval)              return_if((obj == NULL), err_null_ptr, retval)
#define expect_greater(val, lower_bound, retval)  return_if((val <= lower_bound), err_illegal_args, retval)
#define expect_good_malloc(obj, retval)           return_if((obj == NULL), err_bad_malloc, retval)

#define ptr_distance(a, b)                                                                                             \
    ((void *)b > (void *)a ? ((void *)b - (void *)a) : ((void *)a - (void *)b))

#define offset_distance(a, b)                                                                                          \
    (a < b ? b - a : MIN_OFFSET )

#define has_flag(val, flag)                                                                                            \
    ((val & flag) == flag)

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t _ext_header_size(size_t free_space_cap, size_t frame_reg_cap);
static inline size_t _total_header_size(size_t free_space_cap, size_t frame_reg_cap);

static inline void *_seek(const page_t *page, seek_target target);

static inline free_space_register_header_t *_free_space_reg_in(const page_t *page);
static inline memory_range_t *_free_space_reg_get(const page_t *page, size_t pos);
static inline size_t _free_space_reg_len(const page_t *page);
static inline bool _free_space_reg_pop(memory_range_t *range, page_t *page);
static inline bool _free_space_reg_add(page_t *page, offset_t begin, offset_t end);
static inline memory_range_t *_free_space_reg_push(const page_t *page);
static inline bool _free_space_reg_alloc(memory_range_t *range, page_t *page, size_t size, block_positioning strategy);
static inline memory_range_t _free_space_reg_split_off(page_t *page, size_t pos, size_t size);
static inline void _free_space_reg_reorganize(page_t *page);
static inline int _comp_memory_ranges_starts(const void* lhs, const void* rhs);
static inline void _free_space_reg_remove_empty_ranges(page_t *page);
static inline void _free_space_reg_merge_ranges(page_t *page);

static inline void _frame_reg_init(page_t *page, size_t num_frames);
static inline frame_register_header_t *_frame_reg_in(const page_t *page);
static inline offset_t *_frame_reg_inuse_get(const page_t *page, frame_id_t frame_id);
static inline frame_id_t *_frame_reg_freelist_get(const page_t *page, size_t pos);
static inline bool _frame_reg_is_full(const page_t *page);
static inline frame_id_t frame_reg_create_frame(page_t *page, block_positioning strategy, size_t size, size_t capacity);
static inline frame_id_t _frame_reg_freelist_pop(page_t *page);
static inline void _frame_reg_link(page_t *page, frame_id_t frame_id, offset_t frame_offset);

static inline void _payload_store(page_t *page, offset_t offset, const void *data, size_t size);

static inline bool memory_range_overlap(memory_range_t *lhs, memory_range_t *rhs);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(page_id_t id, size_t size, page_flags flags, size_t free_space, size_t frame_reg)
{
    size_t header_size = _total_header_size(free_space, frame_reg);
    size_t min_page_size = header_size + FRAME_HDR_SIZE + MINPAYLOAD_SIZE;

    size_t free_space_begin_offset = header_size;
    size_t free_space_size = (size - header_size);
    size_t free_space_end_offset = free_space_begin_offset + free_space_size;

    warn_if((size < MINPAYLOAD_SIZE), "requested page size %zu bytes is too small. Must be at least %zu bytes.\n",
            size, min_page_size)
    expect_greater(size, min_page_size, NULL)
    expect_greater(flags, 0, NULL)
    expect_greater(free_space, 0, NULL)
    expect_greater(frame_reg, 0, NULL)
    expect_greater(free_space_size, 1, NULL)

    page_t *page = malloc(size);
    expect_good_malloc(page, NULL);
    page->page_header.page_id = id;
    page->page_header.page_size = size;
    page->page_header.free_space = free_space_size;
    page->page_header.flags.is_dirty  = has_flag(flags, page_flag_dirty);
    page->page_header.flags.is_fixed  = has_flag(flags, page_flag_fixed);
    page->page_header.flags.is_locked = has_flag(flags, page_flag_locked);
    page->frame_register.free_list_len = page->frame_register.in_use_num = page->free_space_register.list_len = 0;
    page->frame_register.max_num_frames = frame_reg;
    page->free_space_register.list_max = free_space;

    _frame_reg_init(page, frame_reg);

    _free_space_reg_add(page, free_space_begin_offset, free_space_end_offset);

    return page;
}

/**
 *
 * The functions sets the following error for <code>error_last()</code>:
 * <ul>
 * <li><code>err_null_ptr</code> - the pointer to <code>page</code> is <code>NULL</code></li>
 * <li><code>err_illegal_args</code> - the parameter <code>element_size</code> or <code>element_capacity</code> is zero</li>
 * <li><code>err_limitreached</code> - there is no free frame slot in this page</li>
 * <li><code>err_bad_malloc</code> - memory allocation failed</li>
 * </ul>
 * In addition, the function might propagate errors from a call to assign free space to a new frame:
 * <ul>
 * <li><code>err_no_free_space</code> - there is not enough subsequent free memory to create a frame</li>
 * <li><code>err_internal</code> - for internal errors, e.g., missing switch cases</li>
 * </ul>
 * In case an error occurred, the function returns <code>NULL</code>. Otherwise it returns a <code>fid_t</code>.
 * @param page
 * @param element_size
 * @param element_capacity
 * @return
 */
fid_t *frame_create(page_t *page, block_positioning strategy, size_t element_size, size_t element_capacity)
{
    fid_t *handle = NULL;

    expect_non_null(page, NULL);
    expect_greater(element_size, 0, NULL);
    expect_greater(element_capacity, 0, NULL);

    if (!_frame_reg_is_full(page)) {
        handle = malloc (sizeof(frame_register_header_t));
        expect_good_malloc(handle, NULL);
        if ((handle->frame_id = frame_reg_create_frame(page, strategy, element_size, element_capacity)) == NULL_FRAME_ID) {
            return NULL;
        }
    } else error(err_limitreached);

    return handle;
}

bool frame_delete(fid_t *frame_handle)
{
    panic("Function is not implemented yet: '%s'", "frame_delete");
}

bool page_free(page_t *page)
{
    // TODO
    return true;
}

void page_dump(FILE *out, const page_t *page)
{
    assert (out);
    printf("\n#\n");
    printf("# A page dump was requested by the system.\n");
    printf("# \n");
    if (page) {
        size_t free_space_reg_len = _free_space_reg_in(page)->list_max;
        size_t frame_reg_len = _frame_reg_in(page)->max_num_frames;
        size_t total_header_size = _total_header_size(free_space_reg_len, frame_reg_len);

        free_space_register_header_t *free_space_reg = _free_space_reg_in(page);
        frame_register_header_t *frame_reg = _frame_reg_in(page);

        size_t page_capacity = (page->page_header.page_size - total_header_size);
        printf("# Page (%p), pid=%d, size/capacity/free=%zu/%zu/%zu byte (footprint=%.4f%%, filled=%.4f%%)\n",
               page, page->page_header.page_id,
               page->page_header.page_size,
               page_capacity,
               page->page_header.free_space,
               (page->page_header.page_size - page_capacity)/(float)(page->page_header.page_size) * 100,
               (page_capacity - page->page_header.free_space)/(float)(page_capacity) * 100);
        printf("#\n");
        printf("# Segments:\n");
        printf("# 0x%08x [HEADER]\n", 0);
        printf("# %#010lx  [free space register]\n",     ptr_distance(page, free_space_reg));
        printf("# %#010lx    capacity: %u\n",            ptr_distance(page, &free_space_reg->list_max),
                                                            free_space_reg->list_max);
        printf("# %#010lx    size: %u\n",                ptr_distance(page, &free_space_reg->list_len),
                                                            free_space_reg->list_len);
        printf("# %#010lx  [frame register]\n",          ptr_distance(page, frame_reg));
        printf("# %#010lx    capacity: %u\n",            ptr_distance(page, &frame_reg->max_num_frames),
                                                            frame_reg->max_num_frames);
        printf("# %#010lx    in-use size: %u\n",         ptr_distance(page, &frame_reg->in_use_num),
                                                            frame_reg->in_use_num);
        printf("# %#010lx    free-list size: %u\n",      ptr_distance(page, &frame_reg->free_list_len),
                                                            frame_reg->free_list_len);
        printf("# %#010lx  [free space data]\n",         ptr_distance(page, _free_space_reg_get(page, 0)));
        for (size_t idx = 0; idx < free_space_reg->list_len; idx++) {
            _free_space_reg_get(page, idx);
            memory_range_t *range =  _free_space_reg_get(page, idx);
            printf("# %#010lx    idx=%zu: off_start=%zu, off_end=%zu\n",
                   ptr_distance(page, range), idx, range->begin, range->end);
        }
        if (free_space_reg->list_len < free_space_reg->list_max) {
            printf("# %#010lx    (undefined until %#010lx)\n",
                   ptr_distance(page, _free_space_reg_get(page, free_space_reg->list_len)),
                   ptr_distance(page, _free_space_reg_get(page, free_space_reg->list_max - 1)));
        }

        printf("# %#010lx  [frame register in-use list]\n",       ptr_distance(page, _frame_reg_inuse_get(page, 0)));
        for (frame_id_t frame_id = 0; frame_id < frame_reg_len; frame_id++) {
            offset_t *offset = _frame_reg_inuse_get(page, frame_id);
            printf("# %#010lx    fid=%05u: offset=", ptr_distance(page, offset), frame_id);
            if (*offset != NULL_OFFSET) {
                printf("%#010lx\n", *offset);
            } else
                printf("(unset)\n");
        }

        printf("# %#010lx  [frame register free-list stack]\n",       ptr_distance(page, _frame_reg_freelist_get(page, 0)));
        for (size_t idx = 0; idx < frame_reg->free_list_len; idx++) {
            frame_id_t *frame = _frame_reg_freelist_get(page, idx);
            printf("# %#010lx    pos=%05zu: fid=%u\n", ptr_distance(page, frame), idx, *frame);
        }

        for (size_t idx = frame_reg->free_list_len; idx < frame_reg->max_num_frames; idx++) {
            frame_id_t *frame = _frame_reg_freelist_get(page, idx);
            printf("# %#010lx    pos=%05zu: (unset)\n", ptr_distance(page, frame), idx);
        }

        printf("# %#010lx [PAYLOAD]\n",                   ptr_distance(page, _seek(page, target_free_space_begin)));
        printf("# %#010lx end\n",                       page->page_header.page_size);

    } else {
        printf("# ERR_NULL_POINTER: The system was unable to fetch details: null pointer to page.\n");
    }
    printf("#\n");
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t _ext_header_size(size_t free_space_cap, size_t frame_reg_cap)
{
    return (free_space_cap * sizeof(memory_range_t) + frame_reg_cap * sizeof(offset_t) + frame_reg_cap * sizeof(frame_id_t));
}

static inline size_t _total_header_size(size_t free_space_cap, size_t frame_reg_cap)
{
    return CORE_HDR_SIZE + _ext_header_size(free_space_cap, frame_reg_cap);
}

static inline void *_seek(const page_t *page, seek_target target)
{
    switch (target) {
        case target_free_space_reg:
            return (void *) page + sizeof(page_header_t);
        case target_frame_reg:
            return _seek(page, target_free_space_reg) + sizeof(free_space_register_header_t);
        case target_free_space_data:
            return _seek(page, target_frame_reg) + sizeof(frame_register_header_t);
        case target_frame_reg_inuse_data:
            return _seek(page, target_free_space_data) + sizeof(memory_range_t) * _free_space_reg_in(page)->list_max;
        case target_frame_reg_freelist_data:
            return _seek(page, target_frame_reg_inuse_data) + _frame_reg_in(page)->max_num_frames * sizeof(offset_t);
        case target_free_space_begin:
            return _seek(page, target_frame_reg_freelist_data) + _frame_reg_in(page)->max_num_frames * sizeof(frame_id_t);
        default: panic("Unknown seek target '%d'", target);
    }
}

static inline free_space_register_header_t *_free_space_reg_in(const page_t *page)
{
    return (free_space_register_header_t *) _seek(page, target_free_space_reg);
}

static inline void _frame_reg_init(page_t *page, size_t num_frames)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);

    frame_reg->free_list_len = frame_reg->max_num_frames = num_frames;
    for (size_t idx = 0; idx < frame_reg->free_list_len; idx++) {
        *_frame_reg_freelist_get(page, idx) = frame_reg->free_list_len - idx - 1;
    }

    frame_reg->in_use_num = 0;
    for (size_t idx = 0; idx < frame_reg->max_num_frames; idx++) {
        *_frame_reg_inuse_get(page, idx) = NULL_OFFSET;
    }
}

static inline frame_register_header_t *_frame_reg_in(const page_t *page)
{
    return (frame_register_header_t *) _seek(page, target_frame_reg);
}

static inline memory_range_t *_free_space_reg_get(const page_t *page, size_t pos)
{
    free_space_register_header_t *free_space_reg = _free_space_reg_in(page);
    assert (pos < free_space_reg->list_max);
    return _seek(page, target_free_space_data) + sizeof(memory_range_t) * pos;
}

static inline size_t _free_space_reg_len(const page_t *page)
{
    free_space_register_header_t *free_space_reg = _free_space_reg_in(page);
    return free_space_reg->list_len;
}

static inline bool _free_space_reg_pop(memory_range_t *range, page_t *page)
{
    free_space_register_header_t *free_space_reg = _free_space_reg_in(page);
    size_t len = _free_space_reg_len(page);
    if (len > 0) {
        if (range != NULL) {
            *range = *_free_space_reg_get(page, _free_space_reg_len(page) - 1);
        }
        free_space_reg->list_len--;
        return true;
    } else return false;
}

static inline offset_t *_frame_reg_inuse_get(const page_t *page, frame_id_t frame_id)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    assert (frame_id < frame_reg->max_num_frames);
    return _seek(page, target_frame_reg_inuse_data) + frame_id * sizeof(offset_t);
}

static inline frame_id_t *_frame_reg_freelist_get(const page_t *page, size_t pos)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    assert (pos < frame_reg->max_num_frames);
    return _seek(page, target_frame_reg_inuse_data) + frame_reg->max_num_frames * sizeof(offset_t) +
            pos * sizeof(frame_id_t);
}

static inline bool _frame_reg_is_full(const page_t *page)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    return (frame_reg->free_list_len == 0);
}

static inline frame_id_t frame_reg_create_frame(page_t *page, block_positioning strategy, size_t size, size_t capacity)
{
    assert (!_frame_reg_is_full(page));

    frame_id_t frame_id = NULL_FRAME_ID;
    memory_range_t free_range;
    if ((_free_space_reg_alloc(&free_range, page, FRAME_HDR_SIZE, strategy))) {
        frame_id = _frame_reg_freelist_pop(page);
        vframe_t frame = {
            .start = {
                .offset = MAX_OFFSET,
                .is_far_ptr = false,
                .page_id = page->page_header.page_id
            },
            .elem_capacity = capacity,
            .elem_size = size
        };
        offset_t frame_offset = free_range.begin;
        assert (ptr_distance(free_range.begin, free_range.end) >= sizeof(vframe_t));
        _payload_store(page, free_range.begin, &frame, sizeof(vframe_t));
        _frame_reg_link(page, frame_id, frame_offset);
    }
    return frame_id;
}

static inline frame_id_t _frame_reg_freelist_pop(page_t *page)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    assert (frame_reg->free_list_len > 0);
    return *_frame_reg_freelist_get(page, --frame_reg->free_list_len);
}

static inline void _frame_reg_link(page_t *page, frame_id_t frame_id, offset_t frame_offset)
{
    assert(page);
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    assert(frame_id < frame_reg->max_num_frames);
    *(offset_t *)(_seek(page, target_frame_reg_inuse_data) + frame_id * sizeof(offset_t)) = frame_offset;
}

static inline memory_range_t *_free_space_reg_push(const page_t *page)
{
    memory_range_t *entry = NULL;
    free_space_register_header_t *free_space_stack = _free_space_reg_in(page);
    if (free_space_stack->list_len < free_space_stack->list_max) {
        size_t entry_pos = free_space_stack->list_len++;
        entry = _free_space_reg_get(page, entry_pos);
    }
    return entry;
}

static inline bool _free_space_reg_alloc(memory_range_t *range, page_t *page, size_t size, block_positioning strategy)
{
    assert(page);
    assert(size > 0);
    assert(range);

    range->begin = MAX_OFFSET;
    range->end = MIN_OFFSET;

    const size_t no_such_entry_id = _free_space_reg_len(page);
    size_t entry_id_cursor = no_such_entry_id, best_entry_id = no_such_entry_id;

    switch (strategy) {
        case positioning_first_nomerge: case positioning_first_merge:
            while (entry_id_cursor--) {
                memory_range_t *cursor = _free_space_reg_get(page, entry_id_cursor);
                if (offset_distance(cursor->begin, cursor->end) >= size) {
                    *range = _free_space_reg_split_off(page, entry_id_cursor, size);
                    break;
                }
            }
            break;
        case positioning_smallest_nomerge: case positioning_smallest_merge:
            while (entry_id_cursor--) {
                memory_range_t *cursor = _free_space_reg_get(page, entry_id_cursor);
                size_t cursor_block_size = offset_distance(cursor->begin, cursor->end);
                if (cursor_block_size >= size &&
                    cursor_block_size < offset_distance(range->begin, range->begin)) {
                    range->begin = cursor->begin;
                    range->end = cursor->end;
                    best_entry_id = entry_id_cursor;
                }
            }
            *range = _free_space_reg_split_off(page, best_entry_id, size);
            break;
        case positioning_largest_nomerge: case positioning_largest_merge:
            while (entry_id_cursor--) {
                memory_range_t *cursor = _free_space_reg_get(page, entry_id_cursor);
                size_t cursor_block_size = offset_distance(cursor->begin, cursor->end);
                if (cursor_block_size >= size &&
                    cursor_block_size > offset_distance(range->begin, range->begin)) {
                    range->begin = cursor->begin;
                    range->end = cursor->end;
                    best_entry_id = entry_id_cursor;
                }
            }
            *range = _free_space_reg_split_off(page, best_entry_id, size);
            break;
        default:
            error(err_internal);
            break;
    }

    if (entry_id_cursor != no_such_entry_id) {
        if ((strategy == positioning_first_merge) ||
            (strategy == positioning_smallest_merge) ||
            (strategy == positioning_largest_merge)) {
            _free_space_reg_reorganize(page);
        }
    }

    bool success = (range->begin < range->end);
    if (!success) {
        error(err_no_free_space);
    }

    return success;
}

static inline memory_range_t _free_space_reg_split_off(page_t *page, size_t pos, size_t size)
{
    assert (page);
    free_space_register_header_t *free_space_reg = _free_space_reg_in(page);
    assert (pos < free_space_reg->list_len);
    memory_range_t *range = _free_space_reg_get(page, pos);
    assert (size <= ptr_distance(range->begin, range->end));
    memory_range_t result = {
        .begin = range->begin,
        .end = range->begin + size
    };
    range->begin = result.end;
    return result;
}

static inline void _free_space_reg_reorganize(page_t *page)
{
    assert (page);
    _free_space_reg_remove_empty_ranges(page);
    _free_space_reg_merge_ranges(page);
}

static inline void _free_space_reg_remove_empty_ranges(page_t *page)
{
    assert (page);
    size_t idx = _free_space_reg_len(page);
    while (idx--) {
        memory_range_t *range = _free_space_reg_get(page, idx);
        assert (range->begin <= range->end);
        if (offset_distance(range->begin, range->end) == 0) {
            size_t reg_len = _free_space_reg_len(page);
            if (idx < reg_len) {
                memcpy((void *) range, _free_space_reg_get(page, idx) + 1, (reg_len - idx) * sizeof(memory_range_t));
            }
            bool success = _free_space_reg_pop(NULL, page);
            assert(success);
            idx++;
        }
    }
}

static inline int _comp_memory_ranges_starts(const void* lhs, const void* rhs)
{
    offset_t a = ((memory_range_t *) lhs)->begin;
    offset_t b = ((memory_range_t *) rhs)->begin;
    return (a == b ? 0 : (a < b) ? - 1 : 1);
}

static inline void _free_space_reg_merge_ranges(page_t *page)
{
    assert (page);
    free_space_register_header_t *free_space_reg = _free_space_reg_in(page);
    size_t len = _free_space_reg_len(page);
    vector_t *vector = vector_create(sizeof(memory_range_t), len);

    size_t idx = len;
    while (idx--) {
        memory_range_t *range = _free_space_reg_get(page, idx);
        vector_add(vector, 1, range);
    }

    void *raw_data = vector_get(vector);
    qsort(raw_data, len, sizeof(memory_range_t), _comp_memory_ranges_starts);

    vector_t *stack = vector_create(sizeof(memory_range_t), len);
    vector_add(stack, 1, raw_data);

    for (size_t range_idx = 0; range_idx < len; range_idx++) {
        memory_range_t *current_range = (memory_range_t *)(raw_data + range_idx * sizeof(memory_range_t));
        memory_range_t *stack_top = vector_peek(stack);
        if (memory_range_overlap(current_range, stack_top)) {
            stack_top->end = max(stack_top->end, current_range->end);
        } else vector_add(stack, 1, current_range);
    }

    free_space_reg->list_len = stack->num_elements;
    memcpy(_free_space_reg_get(page, 0), vector_get(stack), stack->num_elements * sizeof(memory_range_t));

    vector_free(vector);
    vector_free(stack);
}

static inline void _payload_store(page_t *page, offset_t offset, const void *data, size_t size)
{
    assert (page);
    assert (offset >= ptr_distance(page, _seek(page, target_free_space_begin)));
    assert (data);
    assert (size > 0);

    void *base = (void *) page;
    memcpy(base + offset, data, size);
}

static inline bool memory_range_overlap(memory_range_t *lhs, memory_range_t *rhs)
{
    return (lhs->end >= rhs->begin && lhs->begin <= rhs->begin) ||
           (rhs->end >= lhs->begin && rhs->begin <= lhs->begin);
}

static inline bool _free_space_reg_add(page_t *page, offset_t begin, offset_t end)
{
    memory_range_t *entry = NULL;
    if ((entry = _free_space_reg_push(page))) {
        expect_non_null(entry, false);
        entry->begin = begin;
        entry->end = end;
    }
    return (entry != NULL);
}