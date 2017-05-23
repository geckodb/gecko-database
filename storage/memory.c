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

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define output_error(code)                                                                          \
    {                                                                                               \
        error(code);                                                                                \
        error_print();                                                                              \
        fprintf(stderr, "\t> occurred here: '%s:%d'\n", __FILE__, __LINE__);                        \
        fflush(stderr);                                                                             \
        exit(1);                                                                                    \
    }

#define return_if(expr, error_code, retval)                                                         \
    if (expr) {                                                                                     \
        output_error(error_code);                                                                   \
        return retval;                                                                              \
    }

#define warn_if(expr, msg, args...)                                                                 \
    if (expr) {                                                                                     \
        fprintf(stderr, "WARNING (%s:%d): ", __FILE__, __LINE__);                                   \
        fprintf(stderr, msg, args);                                                                 \
        fflush(stderr);                                                                             \
    }

#define debug_var(type, name, value)                                                                \
    type name = value;


#define expect_non_null(obj, retval)              return_if((obj == NULL), err_null_ptr, retval)
#define expect_greater(val, lower_bound, retval)  return_if((val <= lower_bound), err_illegal_args, retval)
#define expect_good_malloc(obj, retval)           return_if((obj == NULL), err_bad_malloc, retval)


#define has_flag(val, flag)                                                                         \
    ((val & flag) == flag)

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t _get_core_header_size();
static inline size_t _get_extension_header_size(size_t free_space_cap, size_t frame_reg_cap);
static inline size_t _get_total_header_size(size_t free_space_cap, size_t frame_reg_cap);
static inline size_t _get_min_vframe_size();
static inline void *_seek_beyond_page_header(page_t *page);
static inline void *_seek_beyond_free_space_register(page_t *page);
static inline void *_seek_beyond_frame_register(page_t *page);
static inline stack_header_t *_get_free_space_register(page_t *page);
static inline memory_range_t *_get_free_space_register_entry(page_t *page, size_t pos);
static inline bool _register_free_space(page_t *page, offset_t begin, offset_t end);
static inline memory_range_t *_get_free_space_register_next(page_t *page);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(page_id_t id, size_t size, page_flags flags, size_t free_space, size_t frame_reg)
{
    size_t header_size = _get_total_header_size(free_space, frame_reg);
    size_t min_vframe_size = _get_min_vframe_size();
    size_t min_page_size = header_size + min_vframe_size;

    size_t free_space_begin_offset = header_size + 1;
    size_t free_space_size = (size - free_space_begin_offset);
    size_t free_space_end_offset = free_space_begin_offset + free_space_size;

    warn_if((size < min_page_size), "requested page size %zu bytes is too small. Must be at least %zu bytes.\n",
            size, min_page_size)
    expect_greater (size, min_page_size, NULL)
    expect_greater (flags, 0, NULL)
    expect_greater (free_space, 0, NULL)
    expect_greater (frame_reg, 0, NULL)

    page_t *page = malloc (size);
    expect_good_malloc(page, NULL);
    page->page_header.page_id = id;
    page->page_header.free_space = free_space_size;
    page->page_header.flags.is_dirty  = has_flag(flags, page_flag_dirty);
    page->page_header.flags.is_fixed  = has_flag(flags, page_flag_fixed);
    page->page_header.flags.is_locked = has_flag(flags, page_flag_locked);
    page->frame_register.list_len = page->free_space_register.list_len = 0;
    page->frame_register.list_max = frame_reg;
    page->free_space_register.list_max = frame_reg;
    _register_free_space(page, free_space_begin_offset, free_space_end_offset);

    return page;
}

bool page_free(page_t *page)
{
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline size_t _get_core_header_size()
{
    return (sizeof(page_t) + 2 * sizeof(stack_header_t));
}

static inline size_t _get_extension_header_size(size_t free_space_cap, size_t frame_reg_cap)
{
    return (free_space_cap * sizeof(memory_range_t) + frame_reg_cap * sizeof(offset_t));
}

static inline size_t _get_total_header_size(size_t free_space_cap, size_t frame_reg_cap)
{
    return _get_core_header_size() + _get_extension_header_size(free_space_cap, frame_reg_cap);
}

static inline size_t _get_min_vframe_size()
{
    return (sizeof(vframe_t) + sizeof(zone_header_t) +
            sizeof(zone_ptr)); /* min payload in zone is far pointer to another zone */
}

static inline void *_seek_beyond_page_header(page_t *page)
{
    return (void *) page + sizeof(page_header_t);
}

static inline void *_seek_beyond_free_space_register(page_t *page)
{
    return _seek_beyond_page_header(page) + sizeof(stack_header_t);
}
static inline void *_seek_beyond_frame_register(page_t *page)
{
    return _seek_beyond_free_space_register(page) + sizeof(stack_header_t);
}

static inline stack_header_t *_get_free_space_register(page_t *page)
{
    return (stack_header_t *) _seek_beyond_page_header(page);
}

static inline stack_header_t *_get_frame_register(page_t *page)
{
    return (stack_header_t *) _seek_beyond_free_space_register(page);
}

static inline memory_range_t *_get_free_space_register_entry(page_t *page, size_t pos)
{
    return _seek_beyond_frame_register(page) + sizeof(memory_range_t) * pos;
}

static inline memory_range_t *_get_free_space_register_next(page_t *page)
{
    memory_range_t *entry = NULL;
    stack_header_t *free_space_stack = _get_free_space_register(page);
    if (free_space_stack->list_len < free_space_stack->list_max) {
        size_t entry_pos = free_space_stack->list_len++;
        entry = _get_free_space_register_entry(page, entry_pos);
    }
    return entry;
}

static inline bool _register_free_space(page_t *page, offset_t begin, offset_t end)
{
    memory_range_t *entry = NULL;
    if ((entry = _get_free_space_register_next(page))) {
        expect_non_null(entry, false);
        entry->begin = begin;
        entry->end = end;
    }
    return (entry != NULL);
}