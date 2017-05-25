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

#define has_flag(val, flag)                                                                                            \
    ((val & flag) == flag)

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t _core_header_size();
static inline size_t _ext_header_size(size_t free_space_cap, size_t frame_reg_cap);
static inline size_t _total_header_size(size_t free_space_cap, size_t frame_reg_cap);
static inline size_t _min_frame_size();

static inline void *__seek_free_space_reg(const page_t *page);
static inline void *__seek_frame_reg(const page_t *page);
static inline void *__seek_free_space_data(const page_t *page);
static inline void *__seek_frame_reg_inuse_data(const page_t *page);
static inline void *__seek_frame_reg_freelist_data(const page_t *page);
static inline void *__seek_free_space_begin(const page_t *page);

static inline free_space_register_header_t *_free_space_reg_in(const page_t *page);
static inline memory_range_t *_free_space_reg_get(const page_t *page, size_t pos);
static inline bool _free_space_reg_add(page_t *page, offset_t begin, offset_t end);
static inline memory_range_t *_free_space_reg_push(const page_t *page);

static inline frame_register_header_t *_frame_reg_in(const page_t *page);
static inline offset_t *_frame_reg_inuse_get(const page_t *page, size_t pos);
static inline offset_t *_frame_reg_freelist_get(const page_t *page, size_t pos);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(page_id_t id, size_t size, page_flags flags, size_t free_space, size_t frame_reg)
{
    size_t header_size = _total_header_size(free_space, frame_reg);
    size_t min_vframe_size = _min_frame_size();
    size_t min_page_size = header_size + min_vframe_size;

    size_t free_space_begin_offset = header_size;
    size_t free_space_size = (size - header_size);
    size_t free_space_end_offset = free_space_begin_offset + free_space_size;

    warn_if((size < min_page_size), "requested page size %zu bytes is too small. Must be at least %zu bytes.\n",
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
    _free_space_reg_add(page, free_space_begin_offset, free_space_end_offset);

    return page;
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
        printf("# 0x%08x header\n", 0);
        printf("# %#010lx free space register\n",       ptr_distance(page, _free_space_reg_in(page)));
        printf("# %#010lx frame register\n",            ptr_distance(page, _frame_reg_in(page)));
        printf("# %#010lx free space data\n",           ptr_distance(page, _free_space_reg_get(page, 0)));
        printf("# %#010lx frame register data\n",       ptr_distance(page, _frame_reg_inuse_get(page, 0)));
        printf("# %#010lx frame register free-list\n",  ptr_distance(page, _frame_reg_freelist_get(page, 0)));
        printf("# %#010lx payload\n",                   ptr_distance(page, __seek_free_space_begin(page)));
        printf("# %#010lx end\n",                       page->page_header.page_size);

    } else {
        printf("# ERR_NULL_POINTER: The system was unable to fetch details: null pointer to page.\n");
    }
    printf("#\n");
    exit(0);
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t _core_header_size()
{
    return (sizeof(page_header_t) + sizeof(free_space_register_header_t) + sizeof(frame_register_header_t));
}

static inline size_t _ext_header_size(size_t free_space_cap, size_t frame_reg_cap)
{
    return (free_space_cap * sizeof(memory_range_t) + 2 * frame_reg_cap * sizeof(offset_t));
}

static inline size_t _total_header_size(size_t free_space_cap, size_t frame_reg_cap)
{
    return _core_header_size() + _ext_header_size(free_space_cap, frame_reg_cap);
}

static inline size_t _min_frame_size()
{
    return (sizeof(vframe_t) + sizeof(zone_header_t) +
            sizeof(zone_ptr)); /* min payload in zone is far pointer to another zone */
}

static inline void *__seek_free_space_reg(const page_t *page)
{
    return (void *) page + sizeof(page_header_t);
}

static inline void *__seek_frame_reg(const page_t *page)
{
    return __seek_free_space_reg(page) + sizeof(free_space_register_header_t);
}

static inline void *__seek_free_space_data(const page_t *page)
{
    return __seek_frame_reg(page) + sizeof(frame_register_header_t);
}

static inline void *__seek_frame_reg_inuse_data(const page_t *page)
{
    free_space_register_header_t *free_reg = _free_space_reg_in(page);
    size_t free_register_size = free_reg->list_max;
    return __seek_free_space_data(page) + sizeof(memory_range_t) * free_register_size;
}

static inline void *__seek_frame_reg_freelist_data(const page_t *page)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    size_t inuse_size = frame_reg->max_num_frames;
    return __seek_frame_reg_inuse_data(page) + inuse_size * sizeof(offset_t);
}

static inline free_space_register_header_t *_free_space_reg_in(const page_t *page)
{
    return (free_space_register_header_t *) __seek_free_space_reg(page);
}

static inline frame_register_header_t *_frame_reg_in(const page_t *page)
{
    return (frame_register_header_t *) __seek_frame_reg(page);
}

static inline memory_range_t *_free_space_reg_get(const page_t *page, size_t pos)
{
    free_space_register_header_t *free_space_reg = _free_space_reg_in(page);
    assert (pos < free_space_reg->list_max);
    return __seek_free_space_data(page) + sizeof(memory_range_t) * pos;
}

static inline offset_t *_frame_reg_inuse_get(const page_t *page, size_t pos)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    assert (pos < frame_reg->max_num_frames);
    return __seek_frame_reg_inuse_data(page) + pos * sizeof(offset_t);
}

static inline offset_t *_frame_reg_freelist_get(const page_t *page, size_t pos)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    assert (pos < frame_reg->max_num_frames);
    return __seek_frame_reg_inuse_data(page) + frame_reg->max_num_frames * sizeof(offset_t) +
            pos * sizeof(offset_t);
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

static inline void *__seek_free_space_begin(const page_t *page)
{
    frame_register_header_t *frame_reg = _frame_reg_in(page);
    size_t inuse_size = frame_reg->max_num_frames;
    return __seek_frame_reg_freelist_data(page) + inuse_size * sizeof(offset_t);
}
