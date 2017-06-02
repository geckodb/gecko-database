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

#define NULL_FID        UINT_MAX
#define MAX_OFFSET      SIZE_MAX
#define NULL_OFFSET     0
#define CORE_HDR_SIZE   (sizeof(page_header_t) + sizeof(free_store_t) + sizeof(frame_store_t))
#define FRAME_HDR_SIZE  (sizeof(frame_t))
#define MIN_DATA_SIZE   (sizeof(zone_t) + sizeof(zone_ptr))

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
    (a < b ? b - a : NULL_OFFSET )

#define has_flag(val, flag)                                                                                            \
    ((val & flag) == flag)

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t sizeof_ext_hdr(size_t free_space_cap, size_t frame_reg_cap);
static inline size_t sizeof_total_hdr(size_t free_space_cap, size_t frame_reg_cap);

static inline void *seek(const page_t *page, seek_target target);

static inline free_store_t *free_store_in(const page_t *page);
static inline range_t *free_store_at(const page_t *page, size_t pos);
static inline size_t free_store_len(const page_t *page);
static inline bool free_store_pop(range_t *range, page_t *page);
static inline bool free_store_push(page_t *page, offset_t begin, offset_t end);
static inline range_t *free_store_new(const page_t *page);
static inline bool free_store_bind(range_t *range, page_t *page, size_t size, block_positioning strategy);
static inline range_t free_store_splitoff(page_t *page, size_t pos, size_t size);
static inline void free_store_rebuild(page_t *page);
static inline void free_store_unempty(page_t *page);
static inline void free_store_merge(page_t *page);
static inline frame_id_t frame_store_scan(const page_t *page, frame_state state);

static inline int comp_range_start(const void *lhs, const void *rhs);


static inline void frame_store_init(page_t *page, size_t num_frames);
static inline frame_store_t *frame_store_in(const page_t *page);
static inline offset_t *frame_store_offset_of(const page_t *page, frame_id_t frame_id);
static inline frame_t *frame_store_frame_by_id(const page_t *page, frame_id_t frame_id);
static inline frame_id_t *frame_store_recycle(const page_t *page, size_t pos);
static inline bool frame_store_is_full(const page_t *page);
static inline frame_id_t frame_store_create(page_t *page, block_positioning strategy, size_t size, size_t capacity);
static inline void frame_store_link(page_t *page, frame_id_t frame_id, offset_t frame_offset);

static inline void data_store_write(page_t *page, offset_t offset, const void *data, size_t size);
static inline void *data_store_at(const page_t *page, offset_t offset);

static inline bool range_do_overlap(range_t *lhs, range_t *rhs);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(page_id_t id, size_t size, page_flags flags, size_t free_space, size_t frame_reg)
{
    size_t header_size = sizeof_total_hdr(free_space, frame_reg);
    size_t min_page_size = header_size + FRAME_HDR_SIZE + MIN_DATA_SIZE;

    size_t free_space_begin_offset = header_size;
    size_t free_space_size = (size - header_size);
    size_t free_space_end_offset = free_space_begin_offset + free_space_size;

    warn_if((size < MIN_DATA_SIZE), "requested page size %zu bytes is too small. Must be at least %zu bytes.\n",
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

    frame_store_init(page, frame_reg);
    free_store_push(page, free_space_begin_offset, free_space_end_offset);

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

    if (!frame_store_is_full(page)) {
        handle = malloc (sizeof(frame_store_t));
        expect_good_malloc(handle, NULL);
        if ((handle->frame_id = frame_store_create(page, strategy, element_size, element_capacity)) == NULL_FID) {
            return NULL;
        }
    } else error(err_limitreached);

    return handle;
}

void zone_create(fid_t *frame_handle, size_t num_of_zones)
{

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
        size_t free_space_reg_len = free_store_in(page)->list_max;
        size_t frame_reg_len = frame_store_in(page)->max_num_frames;
        size_t total_header_size = sizeof_total_hdr(free_space_reg_len, frame_reg_len);

        free_store_t *free_space_reg = free_store_in(page);
        frame_store_t *frame_reg = frame_store_in(page);

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
        printf("# %#010lx  [free space data]\n",         ptr_distance(page, free_store_at(page, 0)));
        for (size_t idx = 0; idx < free_space_reg->list_len; idx++) {
            free_store_at(page, idx);
            range_t *range = free_store_at(page, idx);
            printf("# %#010lx    idx=%zu: off_start=%zu, off_end=%zu\n",
                   ptr_distance(page, range), idx, range->begin, range->end);
        }
        if (free_space_reg->list_len < free_space_reg->list_max) {
            printf("# %#010lx    (undefined until %#010lx)\n",
                   ptr_distance(page, free_store_at(page, free_space_reg->list_len)),
                   ptr_distance(page, free_store_at(page, free_space_reg->list_max - 1)));
        }

        printf("# %#010lx  [frame register in-use list]\n",       ptr_distance(page, frame_store_offset_of(page, 0)));
        for (frame_id_t frame_id = 0; frame_id < frame_reg_len; frame_id++) {
            offset_t *offset = frame_store_offset_of(page, frame_id);
            printf("# %#010lx    fid=%05u: offset=", ptr_distance(page, offset), frame_id);
            if (*offset != NULL_OFFSET) {
                printf("%#010lx\n", *offset);
            } else
                printf("(unset)\n");
        }

        printf("# %#010lx  [frame register free-list stack]\n",       ptr_distance(page, frame_store_recycle(page, 0)));
        for (size_t idx = 0; idx < frame_reg->free_list_len; idx++) {
            frame_id_t *frame = frame_store_recycle(page, idx);
            printf("# %#010lx    pos=%05zu: fid=%u\n", ptr_distance(page, frame), idx, *frame);
        }

        for (size_t idx = frame_reg->free_list_len; idx < frame_reg->max_num_frames; idx++) {
            frame_id_t *frame = frame_store_recycle(page, idx);
            printf("# %#010lx    pos=%05zu: (unset)\n", ptr_distance(page, frame), idx);
        }

        printf("# %#010lx [PAYLOAD]\n",                   ptr_distance(page, seek(page, target_free_space_begin)));

        printf("# ---------- [FRAMES IN USE]\n");
        for (frame_id_t frame_id = frame_store_scan(page, frame_inuse); frame_id != NULL_FID;
             frame_id = frame_store_scan(NULL, frame_inuse))
        {
            frame_t *frame = frame_store_frame_by_id(page, frame_id);
            assert (frame);

            offset_t frame_offset = *frame_store_offset_of(page, frame_id);
            printf("# %#010lx    elem_size:%zu, elem_cap:%zu, far_ptr:%d, pid=%d, offset=%#010lx\n",
                       frame_offset, frame->elem_size, frame->elem_capacity, frame->start.is_far_ptr,
                       frame->start.page_id, frame->start.offset);
        }

        printf("# %#010lx end\n",                       page->page_header.page_size);

    } else {
        printf("# ERR_NULL_POINTER: The system was unable to fetch details: null pointer to page.\n");
    }
    printf("#\n");
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t sizeof_ext_hdr(size_t free_space_cap, size_t frame_reg_cap)
{
    return (free_space_cap * sizeof(range_t) + frame_reg_cap * sizeof(offset_t) + frame_reg_cap * sizeof(frame_id_t));
}

static inline size_t sizeof_total_hdr(size_t free_space_cap, size_t frame_reg_cap)
{
    return CORE_HDR_SIZE + sizeof_ext_hdr(free_space_cap, frame_reg_cap);
}

static inline void *seek(const page_t *page, seek_target target)
{
    switch (target) {
        case target_free_space_reg:
            return (void *) page + sizeof(page_header_t);
        case target_frame_reg:
            return seek(page, target_free_space_reg) + sizeof(free_store_t);
        case target_free_space_data:
            return seek(page, target_frame_reg) + sizeof(frame_store_t);
        case target_frame_reg_inuse_data:
            return seek(page, target_free_space_data) + sizeof(range_t) * free_store_in(page)->list_max;
        case target_frame_reg_freelist_data:
            return seek(page, target_frame_reg_inuse_data) + frame_store_in(page)->max_num_frames * sizeof(offset_t);
        case target_free_space_begin:
            return seek(page, target_frame_reg_freelist_data) +
                    frame_store_in(page)->max_num_frames * sizeof(frame_id_t);
        default: panic("Unknown seek target '%d'", target);
    }
}

static inline free_store_t *free_store_in(const page_t *page)
{
    return (free_store_t *) seek(page, target_free_space_reg);
}

static inline void frame_store_init(page_t *page, size_t num_frames)
{
    frame_store_t *frame_reg = frame_store_in(page);

    frame_reg->free_list_len = frame_reg->max_num_frames = num_frames;
    for (size_t idx = 0; idx < frame_reg->free_list_len; idx++) {
        *frame_store_recycle(page, idx) = frame_reg->free_list_len - idx - 1;
    }

    frame_reg->in_use_num = 0;
    for (size_t idx = 0; idx < frame_reg->max_num_frames; idx++) {
        *frame_store_offset_of(page, idx) = NULL_OFFSET;
    }
}

static inline frame_store_t *frame_store_in(const page_t *page)
{
    return (frame_store_t *) seek(page, target_frame_reg);
}

static inline range_t *free_store_at(const page_t *page, size_t pos)
{
    free_store_t *free_space_reg = free_store_in(page);
    assert (pos < free_space_reg->list_max);
    return seek(page, target_free_space_data) + sizeof(range_t) * pos;
}

static inline size_t free_store_len(const page_t *page)
{
    free_store_t *free_space_reg = free_store_in(page);
    return free_space_reg->list_len;
}

static inline bool free_store_pop(range_t *range, page_t *page)
{
    free_store_t *free_space_reg = free_store_in(page);
    size_t len = free_store_len(page);
    if (len > 0) {
        if (range != NULL) {
            *range = *free_store_at(page, free_store_len(page) - 1);
        }
        free_space_reg->list_len--;
        return true;
    } else return false;
}

static inline offset_t *frame_store_offset_of(const page_t *page, frame_id_t frame_id)
{
    assert (page);
    frame_store_t *frame_reg = frame_store_in(page);
    assert (frame_id < frame_reg->max_num_frames);
    return seek(page, target_frame_reg_inuse_data) + frame_id * sizeof(offset_t);
}

static inline frame_t *frame_store_frame_by_id(const page_t *page, frame_id_t frame_id)
{
    assert (page);
    offset_t offset = *frame_store_offset_of(page, frame_id);
    void *data = data_store_at(page, offset);
    expect_non_null(data, NULL);
    return data;
}

static inline frame_id_t *frame_store_recycle(const page_t *page, size_t pos)
{
    frame_store_t *frame_reg = frame_store_in(page);
    assert (pos < frame_reg->max_num_frames);
    return seek(page, target_frame_reg_inuse_data) + frame_reg->max_num_frames * sizeof(offset_t) +
            pos * sizeof(frame_id_t);
}

static inline bool frame_store_is_full(const page_t *page)
{
    frame_store_t *frame_reg = frame_store_in(page);
    return (frame_reg->free_list_len == 0);
}

static inline frame_id_t frame_store_create(page_t *page, block_positioning strategy, size_t size, size_t capacity)
{
    assert (!frame_store_is_full(page));

    frame_id_t frame_id = NULL_FID;
    range_t free_range;
    if ((free_store_bind(&free_range, page, FRAME_HDR_SIZE, strategy))) {
        frame_id = *frame_store_recycle(page, --(frame_store_in(page))->free_list_len);

        frame_t frame = {
            .start = {
                .offset = MAX_OFFSET,
                .is_far_ptr = false,
                .page_id = page->page_header.page_id
            },
            .elem_capacity = capacity,
            .elem_size = size
        };
        offset_t frame_offset = free_range.begin;
        assert (ptr_distance(free_range.begin, free_range.end) >= sizeof(frame_t));
        data_store_write(page, free_range.begin, &frame, sizeof(frame_t));
        frame_store_link(page, frame_id, frame_offset);
    }
    return frame_id;
}

static inline void frame_store_link(page_t *page, frame_id_t frame_id, offset_t frame_offset)
{
    assert(page);
    frame_store_t *frame_reg = frame_store_in(page);
    assert(frame_id < frame_reg->max_num_frames);
    *(offset_t *)(seek(page, target_frame_reg_inuse_data) + frame_id * sizeof(offset_t)) = frame_offset;
}

static inline range_t *free_store_new(const page_t *page)
{
    range_t *entry = NULL;
    free_store_t *free_space_stack = free_store_in(page);
    if (free_space_stack->list_len < free_space_stack->list_max) {
        size_t entry_pos = free_space_stack->list_len++;
        entry = free_store_at(page, entry_pos);
    }
    return entry;
}

static inline bool free_store_bind(range_t *range, page_t *page, size_t size, block_positioning strategy)
{
    assert(page);
    assert(size > 0);
    assert(range);

    range->begin = MAX_OFFSET;
    range->end = NULL_OFFSET;

    const size_t no_such_entry_id = free_store_len(page);
    size_t entry_id_cursor = no_such_entry_id, best_entry_id = no_such_entry_id;

    switch (strategy) {
        case positioning_first_nomerge: case positioning_first_merge:
            while (entry_id_cursor--) {
                range_t *cursor = free_store_at(page, entry_id_cursor);
                if (offset_distance(cursor->begin, cursor->end) >= size) {
                    *range = free_store_splitoff(page, entry_id_cursor, size);
                    break;
                }
            }
            break;
        case positioning_smallest_nomerge: case positioning_smallest_merge:
            while (entry_id_cursor--) {
                range_t *cursor = free_store_at(page, entry_id_cursor);
                size_t cursor_block_size = offset_distance(cursor->begin, cursor->end);
                if (cursor_block_size >= size &&
                    cursor_block_size < offset_distance(range->begin, range->begin)) {
                    range->begin = cursor->begin;
                    range->end = cursor->end;
                    best_entry_id = entry_id_cursor;
                }
            }
            *range = free_store_splitoff(page, best_entry_id, size);
            break;
        case positioning_largest_nomerge: case positioning_largest_merge:
            while (entry_id_cursor--) {
                range_t *cursor = free_store_at(page, entry_id_cursor);
                size_t cursor_block_size = offset_distance(cursor->begin, cursor->end);
                if (cursor_block_size >= size &&
                    cursor_block_size > offset_distance(range->begin, range->begin)) {
                    range->begin = cursor->begin;
                    range->end = cursor->end;
                    best_entry_id = entry_id_cursor;
                }
            }
            *range = free_store_splitoff(page, best_entry_id, size);
            break;
        default:
            error(err_internal);
            break;
    }

    if (entry_id_cursor != no_such_entry_id) {
        if ((strategy == positioning_first_merge) ||
            (strategy == positioning_smallest_merge) ||
            (strategy == positioning_largest_merge)) {
            free_store_rebuild(page);
        }
    }

    bool success = (range->begin < range->end);
    if (!success) {
        error(err_no_free_space);
    }

    return success;
}

static inline range_t free_store_splitoff(page_t *page, size_t pos, size_t size)
{
    assert (page);
    free_store_t *free_space_reg = free_store_in(page);
    assert (pos < free_space_reg->list_len);
    range_t *range = free_store_at(page, pos);
    assert (size <= ptr_distance(range->begin, range->end));
    range_t result = {
        .begin = range->begin,
        .end = range->begin + size
    };
    range->begin = result.end;
    page->page_header.free_space -= size;
    return result;
}

static inline void free_store_rebuild(page_t *page)
{
    assert (page);
    free_store_unempty(page);
    free_store_merge(page);
}

static inline void free_store_unempty(page_t *page)
{
    assert (page);
    size_t idx = free_store_len(page);
    while (idx--) {
        range_t *range = free_store_at(page, idx);
        assert (range->begin <= range->end);
        if (offset_distance(range->begin, range->end) == 0) {
            size_t reg_len = free_store_len(page);
            if (idx < reg_len) {
                memcpy((void *) range, free_store_at(page, idx) + 1, (reg_len - idx) * sizeof(range_t));
            }
            bool success = free_store_pop(NULL, page);
            assert(success);
            idx++;
        }
    }
}

static inline int comp_range_start(const void *lhs, const void *rhs)
{
    offset_t a = ((range_t *) lhs)->begin;
    offset_t b = ((range_t *) rhs)->begin;
    return (a == b ? 0 : (a < b) ? - 1 : 1);
}

static inline void free_store_merge(page_t *page)
{
    assert (page);
    free_store_t *free_space_reg = free_store_in(page);
    size_t len = free_store_len(page);
    vector_t *vector = vector_create(sizeof(range_t), len);

    size_t idx = len;
    while (idx--) {
        range_t *range = free_store_at(page, idx);
        vector_add(vector, 1, range);
    }

    void *raw_data = vector_get(vector);
    qsort(raw_data, len, sizeof(range_t), comp_range_start);

    vector_t *stack = vector_create(sizeof(range_t), len);
    vector_add(stack, 1, raw_data);

    for (size_t range_idx = 0; range_idx < len; range_idx++) {
        range_t *current_range = (range_t *)(raw_data + range_idx * sizeof(range_t));
        range_t *stack_top = vector_peek(stack);
        if (range_do_overlap(current_range, stack_top)) {
            stack_top->end = max(stack_top->end, current_range->end);
        } else vector_add(stack, 1, current_range);
    }

    free_space_reg->list_len = stack->num_elements;
    memcpy(free_store_at(page, 0), vector_get(stack), stack->num_elements * sizeof(range_t));

    vector_free(vector);
    vector_free(stack);
}

static inline frame_id_t frame_store_scan(const page_t *page, frame_state state)
{
    static size_t frame_id;
    if (page != NULL) {
        frame_id = frame_store_in(page)->max_num_frames;
    }

    while (frame_id--) {
        offset_t frame_offset = *frame_store_offset_of(page, frame_id);
        if ((state == frame_inuse && frame_offset != NULL_OFFSET) ||
            (state == frame_free  && frame_offset == NULL_OFFSET))
            return frame_id;
    }

    return NULL_FID;
}

static inline void data_store_write(page_t *page, offset_t offset, const void *data, size_t size)
{
    assert (page);
    assert (offset >= ptr_distance(page, seek(page, target_free_space_begin)));
    assert (data);
    assert (size > 0);

    void *base = (void *) page;
    memcpy(base + offset, data, size);
}

static inline void *data_store_at(const page_t *page, offset_t offset)
{
    assert (page);
    assert (offset >= ptr_distance(page, seek(page, target_free_space_begin)));
    return offset != NULL_OFFSET ? (void *) page + offset : NULL;
}

static inline bool range_do_overlap(range_t *lhs, range_t *rhs)
{
    return (lhs->end >= rhs->begin && lhs->begin <= rhs->begin) ||
           (rhs->end >= lhs->begin && rhs->begin <= lhs->begin);
}

static inline bool free_store_push(page_t *page, offset_t begin, offset_t end)
{
    range_t *entry = NULL;
    if ((entry = free_store_new(page))) {
        expect_non_null(entry, false);
        entry->begin = begin;
        entry->end = end;
    }
    return (entry != NULL);
}