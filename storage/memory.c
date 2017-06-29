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

#include <limits.h>

#include <error.h>
#include <macros.h>
#include <msg.h>
#include <require.h>
#include <storage/memory.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define NULL_FID        UINT_MAX
#define MAX_OFFSET      SIZE_MAX
#define NULL_OFFSET     0
#define CORE_HDR_SIZE   (sizeof(page_header_t) + sizeof(freespace_reg_t) + sizeof(lane_reg_t))
#define FRAME_HDR_SIZE  (sizeof(lane_t))
#define MIN_DATA_SIZE   (sizeof(zone_t) + sizeof(in_page_ptr))

#define BADBRANCH        "Internal error: unknown condition during operation on object %p."
#define BADCAST          "Unsupported cast request for persistent ptr %p."
#define BADTYPE          "Persistent ptr %p points to illegal type."
#define BADBACKLINK      "Internal error: back link for persistent ptr %p is not allowed to be null."
#define BADFRWDLINK      "Internal error: forward link for persistent ptr %p is not allowed to point to a non-zone obj."
#define BADPOOLINIT      "Internal error: page pool %p is already initialized."
#define BADPAGEID        "Internal error: page id '%du' was not loaded into hot store. Maybe it does not exists."
#define BADARGNULL       "Illegal argument for function. Parameter '%s' points to null"
#define BADARGZERO       "Illegal argument for function. Parameter '%s' is zero"
#define UNEXPECTED       "Unexpected behavior: '%s'"
#define BADHURRY         "Not implemented: '%s'"
#define BADPAGESIZE      "Request to create a page size of %zuB with a hot store limit of %zuB is illegal"
#define BADSTATE         "Bad state: %s"
#define BADCOLDSTOREINIT "Memory allocation failed: unable to initialize cold store of buffer manager"
#define BADFREELISTINIT  "Memory allocation failed: unable to initialize page free list in buffer manager"
#define BADHOTSTOREOBJ   "Page with id '%du' is not located in hot store"
#define BADINTERNAL      "Internal error: %s"

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef enum {
    SEEK_FREESPACE_REGISTER,
    SEEK_FREESPACE_ENTRIES,
    SEEK_LANE_REGISTER,
    SEEK_LANE_INUSE,
    SEEK_LANE_FREELIST,
    SEEK_PAYLOAD
} seek_target;

typedef enum {
    PAGE_PTR_NEAR,
    PAGE_PTR_FAR
} page_ptr_type;

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define ERROR_OUT(code)                                                                                                \
    {                                                                                                                  \
        error(code);                                                                                                   \
        error_print();                                                                                                 \
        fprintf(stderr, "\t> occurred here: '%s:%d'\n", __FILE__, __LINE__);                                           \
        fflush(stderr);                                                                                                \
        abort();                                                                                                       \
    }

#define RETURN_IF(expr, error_code, retval)                                                                            \
    if (expr) {                                                                                                        \
        ERROR_OUT(error_code);                                                                                         \
        return retval;                                                                                                 \
    }

#define ARG_NUM(...)                                                                                                   \
    (sizeof((int[]){__VA_ARGS__})/sizeof(int))

#define IN_RANGE(type, val, ...)                                                                                       \
    ({                                                                                                                 \
        bool retval = false;                                                                                           \
        type *list = (type[]){__VA_ARGS__};                                                                            \
        for (unsigned i = 0; i < ARG_NUM(__VA_ARGS__); i++)                                                            \
            if (*(list + i) == val) {                                                                                  \
                retval = true;                                                                                         \
                break;                                                                                                 \
            }                                                                                                          \
        retval;                                                                                                        \
    })

#define EXPECT_NONNULL(obj, retval)               RETURN_IF((obj == NULL), err_null_ptr, retval)
#define EXPECT_GREATER(val, lower_bound, retval)  RETURN_IF((val <= lower_bound), err_illegal_args, retval)
#define EXPECT_GOOD_MALLOC(obj, retval)           RETURN_IF((obj == NULL), err_bad_malloc, retval)

#define PTR_DISTANCE(a, b)                                                                                             \
    ((void *)b > (void *)a ? ((void *)b - (void *)a) : ((void *)a - (void *)b))

#define OFFSET_DISTANCE(a, b)                                                                                          \
    (a < b ? b - a : NULL_OFFSET )

#define HAS_FLAG(val, flag)                                                                                            \
    ((val & flag) == flag)

#define PAGE_NULL_PTR()                                                                                                \
    (in_page_ptr) {                                                                                                    \
        .offset = NULL_OFFSET,                                                                                         \
        .target_type_bit_0 = 1,                                                                                        \
        .target_type_bit_1 = 1,                                                                                        \
        .is_far_ptr = false,                                                                                           \
        .page_id = 0                                                                                                   \
    }

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

// - B A S I C ---------------------------------------------------------------------------------------------------------

static inline void *
seek(
        const page_t *    page,
        seek_target       target
);

static inline void
write_unsafe(
        page_t *          page,
        offset_t          offset,
        const void *      data,
        size_t            size
);

static inline void *
read_unsafe(
        const page_t *    page,
        offset_t          offset);

// - F R E E S P A C E   R E G I S T E R -------------------------------------------------------------------------------

static inline freespace_reg_t *
freespace(
        const page_t *    page
);

static inline range_t *
freespace_at(
        const page_t *    page,
        size_t            pos
);

static inline size_t
freespace_len(
        const page_t *    page
);

static inline bool
freespace_pop(
        range_t *         range,
        page_t *          page
);

static inline bool
freespace_push(
        page_t *          page,
        offset_t          begin,
        offset_t          end
);

static inline range_t *
freespace_new(
        const page_t *    page
);

static inline int
freespace_comp_by_start(
        const void *      lhs,
        const void *      rhs
);

static inline bool
freespace_bind(
        range_t *         range,
        page_t *          page,
        size_t            size,
        block_pos         strategy
);

static inline range_t
freespace_split(
        page_t *          page,
        size_t            pos,
        size_t            size
);

static inline void
freespace_rebuild(
        page_t *          page
);

static inline void
freespace_cleanup(
        page_t *          page
);

static inline void
freespace_merge(
        page_t *          page
);

static inline size_t
freespace_find_first(
        const page_t *    page,
        size_t            capacity
);

static inline size_t
freespace_largest(
        const page_t *    page
);

// - P A G E -----------------------------------------------------------------------------------------------------------

static inline size_t
page_approx_freespace(
        const page_t *    page
);

static inline void
page_approx_freespace_inc(
        page_t *          page,
        size_t            size
);

static inline void
page_approx_freespace_dec(
        page_t *          page,
        size_t            size
);

static inline size_t
page_ext_header_sizeof(
        size_t            free_space_cap,
        size_t            frame_reg_cap
);

static inline size_t
page_total_header_sizeof(
        size_t            free_space_cap,
        size_t            frame_reg_cap
);

// - P A G E  P O I N T E R --------------------------------------------------------------------------------------------

static inline bool
ptr_is_null(
        const in_page_ptr *ptr
);

static inline bool
ptr_has_scope(
        const in_page_ptr *ptr,
        ptr_scope_type     type
);

static inline void
ptr_make(
        in_page_ptr *      ptr,
        page_t *           page,
        page_ptr_type      type,
        ptr_target         target,
        offset_t           offset
);

static inline ptr_target
ptr_typeof(
        const in_page_ptr *ptr
);

static inline void *
ptr_deref(
        anit_buffer_t * buffer_manager,
        const in_page_ptr *ptr
);

static inline zone_t *
ptr_cast_zone(
        anit_buffer_t * buffer_manager,
        const in_page_ptr *ptr
);

static inline lane_t *
ptr_cast_frame(
        anit_buffer_t * buffer_manager,
        const in_page_ptr *ptr
);

// - L A N E   R E G I S T E R  ----------------------------------------------------------------------------------------

static inline lane_t *
lane_by_zone(
        anit_buffer_t *manager,
        const zone_t     *zone
);

static inline void
lane_reg_init(
        page_t *          page,
        size_t            num_frames
);

static inline lane_reg_t *
lane_reg(
        const page_t *    page
);

static inline offset_t *
lane_offsetof(
        const page_t *    page,
        lane_id_t         id
);

static inline lane_t *
lane_by_id(
        const page_t *   page,
        lane_id_t        id
);

static inline lane_id_t *
lane_reg_entry_by_pos(
        const page_t *    page,
        size_t            pos
);

static inline bool
lane_reg_is_full(
        const page_t *    page
);

static inline lane_id_t
lane_new(
        page_t *          page,
        block_pos         strategy,
        size_t            size
);

static inline void
lane_reg_link(
        page_t *          page,
        lane_id_t         id,
        offset_t          offset
);

static inline lane_id_t
lane_reg_scan(
        const page_t *    page,
        frame_state       state
);



static inline bool
range_do_overlap(
        range_t *         lhs,
        range_t *         rhs);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

anit_buffer_t *buf_create(
        size_t page_size,
        size_t free_space_reg_capacity,
        size_t frame_reg_capacity,
        size_t max_hot_store_size)
{
    EXPECT_GREATER(page_size, 0, NULL);
    EXPECT_GREATER(max_hot_store_size, 0, NULL);
    EXPECT_GREATER(free_space_reg_capacity, 0, NULL);
    EXPECT_GREATER(frame_reg_capacity, 0, NULL);

    panic_if((max_hot_store_size < 2 * page_size), BADPAGESIZE, page_size, max_hot_store_size);

    anit_buffer_t *result = malloc(sizeof(anit_buffer_t));
    EXPECT_GOOD_MALLOC(result, NULL);
    result->page_anticache.page_hot_store = fixed_linear_hash_table_create(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                                           sizeof(page_id_t), sizeof(void*), HOTSTORE_INITCAP,
                                                           HOTSTORE_GROW_FACTOR, HOTSTORE_MAX_FILL_FAC);
    result->max_size_hot_store = max_hot_store_size;
    EXPECT_NONNULL(result->page_anticache.page_hot_store, NULL);

    result->config = (buffer_manager_config_t) {
        .frame_reg_capacity = frame_reg_capacity,
        .free_space_reg_capacity = free_space_reg_capacity,
        .page_size = page_size
    };

    page_anticache_init(result);
    page_cold_store_init(result);

    return result;
}

static inline page_t *generic_store_get(
    anit_buffer_t    *manager,
    size_t               size,
    const page_id_t     *(*iterate)(anit_buffer_t *, const page_id_t*),
    page_t              *(*fetch)(anit_buffer_t *, page_id_t),
    size_t              (*get_capacity)(page_t *, free_space_get_strategy)
)
{
    const page_id_t *cursor = NULL;
    page_t *page = NULL;

    while ((cursor = iterate(manager, cursor)) != NULL) {
        page = fetch(manager, *cursor);
        size_t quick_free = get_capacity(page, free_space_get_quickapprox);
        if (quick_free >= size) {
            size_t exact_free = get_capacity(page, free_space_get_slowexact);
            if (exact_free >= size)
                return page;
        }
    }

    return NULL;
}

static inline page_t *page_anticache_find_page_by_free_size(
    anit_buffer_t    *manager,
    size_t               size,
    page_t              *favored_page)
{
    page_t *page = NULL;

    if ((favored_page != NULL) && (page_get_free_space(favored_page, free_space_get_quickapprox) >= size) &&
        (page_get_free_space(favored_page, free_space_get_slowexact) >= size)) {
        page = favored_page;
    } else {
        if (!page_anticache_hot_store_is_empty(manager)) {
            page = generic_store_get(manager, size, page_anticache_hot_store_iterate, page_hot_store_fetch,
                                     page_get_free_space);
        }

        if (page == NULL && !page_anticache_cold_store_is_empty(manager)) {
            page = generic_store_get(manager, size, page_anticache_cold_store_iterate, page_cold_store_fetch, page_get_free_space);
            page_anticache_hot_store_add(manager, page);
        }

        if (page == NULL) {
            page = page_anticache_create_page(manager);
            //page_anticache_hot_store_add(manager, page);
        }
    }

    return page;
}

static inline void page_anticache_pin_page(
    anit_buffer_t    *manager,
    page_t              *page)
{
    warn(NOTIMPLEMENTED, to_string(page_anticache_pin_page));   // TODO: Implement
    printf("DEBUG: PIN PAGE request was for page id %d\n", page->page_header.page_id);
}

static inline void page_anticache_unpin_page(
    anit_buffer_t    *manager,
    page_t              *page)
{
    warn(NOTIMPLEMENTED, to_string(page_anticache_unpin_page));   // TODO: Implement
    printf("DEBUG: UNPIN PAGE request was for page id %d\n", page->page_header.page_id);
}

static inline lane_id_t page_anticache_create_frame(
    anit_buffer_t    *manager,
    page_t              *page,
    block_pos    strategy,
    size_t               size
) {
    require_non_null(manager);
    require_non_null(page);
    require((size > 0), "size must be non-zero");
    panic_if(!(page_hot_store_has(manager, page->page_header.page_id)), BADHOTSTOREOBJ, page->page_header.page_id);
    fid_t *fid = frame_create(page, strategy, size);            // TODO: just return frame_id here instead of frame handle
    panic_if((fid == NULL), UNEXPECTED, "Frame handle is null");
    lane_id_t result = fid->frame_id;
    free (fid);
    return result;
}

cursor_t *buf_alloc(
        anit_buffer_t *manager,
        size_t size, size_t nlanes,
        block_pos strategy)
{
    EXPECT_NONNULL(manager, NULL);
    EXPECT_GREATER(size, 0, NULL);
    EXPECT_GREATER(nlanes, 0, NULL);

    cursor_t *result = require_good_malloc(sizeof(cursor_t));
    page_t *page_frame = page_anticache_find_page_by_free_size(manager, FRAME_HDR_SIZE, NULL);
    lane_id_t frame = page_anticache_create_frame(manager, page_frame, strategy, size);

    page_anticache_pin_page(manager, page_frame);
    page_t *last_page_written_to = page_frame;

    while (nlanes--) {
        page_t *page_zone = page_anticache_find_page_by_free_size(manager, size, last_page_written_to);
        last_page_written_to = page_zone;
        printf("DEBUG: page %d capacity left: %zuB\n", page_zone->page_header.page_id, page_zone->page_header.free_space);
        zone_t *new_zone = NULL;

        page_anticache_pin_page(manager, page_zone);
        nlanes += ((new_zone = buf_zone_create(manager, page_frame, frame, page_zone, strategy)) == NULL);
        page_anticache_unpin_page(manager, page_zone);
    }

    page_anticache_unpin_page(manager, page_frame);

    *result = (cursor_t) {
        .page_id   = page_frame->page_header.page_id,
        .frame_id  = frame,
        .manager   = manager,
        .zone      = NULL,
        .state     = block_state_closed,
    };

    return result;
}

void buf_cursor_free(
        cursor_t *cursor)
{
    require_non_null(cursor);
    assert ((cursor->state != block_state_opened) || (cursor->zone != NULL));
    free (cursor);
}

zone_id_t buffer_manager_block_nextzone(
    cursor_t *ptr)
{
    panic(NOTIMPLEMENTED, to_string(buffer_manager_block_nextzone));   // TODO: Implement
    return 0;
}

void buffer_manager_block_setzones(
    cursor_t *ptr,
    zone_id_t last_zone)
{
    panic(NOTIMPLEMENTED, to_string(buffer_manager_block_setzones));   // TODO: Implement
}

void buffer_manager_block_rmzone(
    cursor_t *ptr,
    zone_id_t zone)
{
    panic(NOTIMPLEMENTED, to_string(buffer_manager_block_rmzone));   // TODO: Implement
}

void buf_open(
        cursor_t *ptr)
{
    require_non_null(ptr);
    require_non_null(ptr->manager);
    panic_if((ptr->state == block_state_opened), BADSTATE, "block was already opened.")

    //page_anticache_activate_page_by_id(ptr->manager, ptr->page_id);           // TODO: Uncomment
    page_t *page = page_anticache_get_page_by_id(ptr->manager, ptr->page_id);   // TODO: Remove
    page_dump(stdout, ptr->manager, page, false);                                      // TODO: Remove

    ptr->state = block_state_opened;
}

bool buf_next(
        cursor_t *ptr)
{
    require_non_null(ptr);
    require_non_null(ptr->manager);

    if (ptr->state == block_state_opened) {
        page_t *page = page_anticache_get_page_by_id(ptr->manager, ptr->page_id);
        if (ptr->zone == NULL) {
            ptr->zone = buf_zone_head(ptr->manager, page, ptr->frame_id);
        } else {
            ptr->zone = buf_zone_next(ptr->manager, ptr->zone);
            ptr->state = (ptr->zone == NULL ? block_state_closed : block_state_opened);
        }
        return (ptr->zone != NULL ? true : false);
    } else return false;
}

void buf_close(
        cursor_t *ptr)
{
    require_non_null(ptr);
    ptr->state = block_state_closed;
}

void buf_read(
        cursor_t *ptr,
        void *capture,
        void (*consumer)(void *capture, const void *data))
{
    require_non_null(ptr);
    require_non_null(consumer);
    panic_if((ptr->zone == NULL), BADSTATE, "read operation on illegal zone");
    consumer (capture, zone_get_data(ptr->zone));
}

void buf_memcpy(
        cursor_t *dst,
        size_t offset,
        const void *src, size_t num)
{
    require_non_null(dst);
    require_non_null(src);
    panic_if((dst->state != block_state_opened), BADSTATE, "block must be opened before call to memcpy");
    panic_if((dst->zone == NULL), BADINTERNAL, "block is opened but pointer to zone is null");
    assert (page_hot_store_has(dst->manager, dst->page_id));
    assert (offset + num <= lane_by_id(page_hot_store_fetch(dst->manager, dst->page_id), dst->frame_id)->elem_size);
    zone_memcpy(dst->zone, offset, src, num);
}

bool buf_free(
        anit_buffer_t *manager)
{
    EXPECT_NONNULL(manager, false);
    EXPECT_NONNULL(manager->page_anticache.page_hot_store, false);
    bool free_page_reg = dict_free(manager->page_anticache.page_hot_store);
    if (free_page_reg) {
        free (manager);
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

// Note: no "flush list"

void page_anticache_init(
    anit_buffer_t *manager)
{
    require_non_null(manager);
    manager->page_anticache.page_hot_store_page_ids = list_create(sizeof(page_id_t));
    manager->page_anticache.cold_store_page_ids = vector_create_ex(sizeof(page_id_t),
                                                                   ANTICACHE_COLDSTORELIST_INITCAP,
                                                                   auto_resize,
                                                                   ANTICACHE_COLDSTORELIST_GROW_FACTOR);
    manager->page_anticache.free_page_ids_stack = vector_create_ex(sizeof(page_id_t),
                                                                   ANTICACHE_PAGEID_FREELIST_INITCAP,
                                                                   auto_resize,
                                                                   ANTICACHE_PAGEID_FREELIST_GROW_FACTOR);
    manager->page_anticache.next_page_id = 0;
    require((manager->page_anticache.page_hot_store_page_ids), BADCOLDSTOREINIT);
    require((manager->page_anticache.cold_store_page_ids), BADCOLDSTOREINIT);
    require((manager->page_anticache.free_page_ids_stack), BADFREELISTINIT);
}

void page_anticache_page_new(
    anit_buffer_t *manager,
    size_t size)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_page_new));   // TODO: Implement
}

page_t *page_anticache_get_page_by_id(anit_buffer_t *manager, page_id_t page_id)
{
    require_non_null(manager);
    warn_if((page_id >= manager->page_anticache.next_page_id), "Requested page id '%d' might not from this pool", page_id);
    page_t *result = NULL;

    if (((result = page_hot_store_fetch(manager, page_id)) == NULL) &&
        ((result = page_cold_store_fetch(manager, page_id)) == NULL)) {
        panic(BADPAGEID, page_id);
    }

    return result;
}

void page_anticache_activate_page_by_id(
    anit_buffer_t *manager,
    page_id_t page_id)
{
    page_anticache_get_page_by_id(manager, page_id);
}

void page_anticache_page_delete(
    anit_buffer_t *manager,
    page_t *page)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_page_delete));   // TODO: Implement
}

void page_anticache_free_list_push(
    anit_buffer_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_free_list_push));   // TODO: Implement
}

bool page_anticache_free_list_is_empty(
    anit_buffer_t *manager)
{
    assert (manager);
    return (manager->page_anticache.free_page_ids_stack->num_elements == 0);
}

page_t *page_anticache_create_page(
    anit_buffer_t *manager)
{
    require_non_null(manager);
    page_t *result = NULL;

    size_t id = !page_anticache_free_list_is_empty(manager) ? page_anticache_free_list_pop(manager) :
                                                              page_anticache_new_page_id(manager);

    result = page_create(manager, id, manager->config.page_size, PAGE_FLAG_FRESH_PAGE_FLAGS,
                         manager->config.free_space_reg_capacity,
                         manager->config.frame_reg_capacity);

    page_anticache_hot_store_add(manager, result);
    return result;
}

page_id_t page_anticache_free_list_pop(
    anit_buffer_t *manager)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_free_list_pop));   // TODO: Implement
}

size_t page_anticache_new_page_id(anit_buffer_t *manager)
{
    require_non_null(manager);
    return manager->page_anticache.next_page_id++;
}

void page_anticache_hot_store_add(
    anit_buffer_t *manager,
    page_t *page)
{
    warn(NOTIMPLEMENTED, to_string(page_anticache_hot_store_add));   // TODO: Implement

    // and register
    page_hot_store_set(manager, page->page_header.page_id, page);

}

void page_anticache_hot_store_remove(
    anit_buffer_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_hot_store_remove));   // TODO: Implement
}

const page_id_t* page_anticache_hot_store_iterate(
    anit_buffer_t *manager,
    const page_id_t* last)
{
    return (last == NULL? list_begin(manager->page_anticache.page_hot_store_page_ids) :
                          list_next(last));
}

bool page_anticache_hot_store_is_empty(
    anit_buffer_t *manager)
{
    assert (manager);

    return (dict_empty(manager->page_anticache.page_hot_store));
}

void page_anticache_cold_store_add(
    anit_buffer_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_cold_store_add));   // TODO: Implement
}

void page_anticache_cold_store_remove(
    anit_buffer_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_cold_store_remove));   // TODO: Implement
}

const page_id_t *page_anticache_cold_store_iterate(
    anit_buffer_t *manager,
    const page_id_t *last)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_cold_store_iterate));   // TODO: Implement
}

bool page_anticache_cold_store_is_empty(
    anit_buffer_t *manager)
{
    assert (manager);
    return (manager->page_anticache.cold_store_page_ids->num_elements == 0);
}

void page_anticache_free(
    anit_buffer_t *manager)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_free));   // TODO: Implement
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool page_hot_store_init(
    anit_buffer_t *manager)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager == NULL), BADARGZERO, to_string(num_pages));
    panic_if(!dict_empty(manager->page_anticache.page_hot_store), BADPOOLINIT, manager);
    return true;
}

void page_hot_store_set(
    anit_buffer_t *manager,
    page_id_t id,
    void *ptr)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager->page_anticache.page_hot_store == NULL), UNEXPECTED, "page register in manager");
  //  warn_if((page_hot_store_has(manager, id)), UNEXPECTED, "page is already registered in hot store");    // TODO: Remove
    dict_put(manager->page_anticache.page_hot_store, &id, &ptr);
    list_push(manager->page_anticache.page_hot_store_page_ids, &id);
    printf("DEBUG: PUT page id %d to hot store\n", id);
}

bool page_hot_store_has(
    anit_buffer_t *manager,
    page_id_t id)
{
    return (page_hot_store_fetch(manager, id) != NULL);
}

bool page_hot_store_remove(
    anit_buffer_t *manager,
    page_id_t id)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager->page_anticache.page_hot_store == NULL), UNEXPECTED, "page register in manager");
    panic_if (!(dict_contains_key(manager->page_anticache.page_hot_store, &id)), BADPAGEID, id);

    const void *it = list_begin(manager->page_anticache.page_hot_store_page_ids);
    do {
        if (* (page_id_t *)it == id) {
            list_remove(it);
            break;
        }
    } while ((it = list_next(it)));

    return dict_remove(manager->page_anticache.page_hot_store, 1, &id);
}

page_t *page_hot_store_fetch(
        anit_buffer_t *manager,
        page_id_t id)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager->page_anticache.page_hot_store == NULL), UNEXPECTED, "page register in manager");

    const void *page_ptr = dict_get(manager->page_anticache.page_hot_store, &id);
    void **element = (void **)page_ptr;

    assert (element != NULL); // TODO: Remove if cold store works

    return (element != NULL ? *element : NULL);
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void page_cold_store_init(
    anit_buffer_t *manager)
{

}

void page_cold_store_clear(
    anit_buffer_t *manager)
{
    panic(NOTIMPLEMENTED, to_string(page_cold_store_clear));   // TODO: Implement
}

page_t *page_cold_store_fetch(
    anit_buffer_t *manager,
    page_id_t id)
{
    assert (manager);
    page_t *loaded_page = manager->page_anticache.cold_store.fetch(id);
    page_anticache_hot_store_add(manager, loaded_page);
    return loaded_page;
}

void page_cold_store_push(
    anit_buffer_t *manager,
    page_t *page)
{
    panic(NOTIMPLEMENTED, to_string(page_cold_store_push));   // TODO: Implement
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(
    anit_buffer_t *manager,
    page_id_t id,
    size_t size,
    page_flags flags,
    size_t free_space,
    size_t frame_reg)
{
    size_t header_size = page_total_header_sizeof(free_space, frame_reg);
    size_t min_page_size = header_size + FRAME_HDR_SIZE + MIN_DATA_SIZE;

    size_t free_space_begin_offset = header_size;
    size_t free_space_size = (size - header_size);
    size_t free_space_end_offset = free_space_begin_offset + free_space_size;

    warn_if((size < MIN_DATA_SIZE), "requested page page_size %zu bytes is too small. Must be at least %zu bytes.\n",
            size, min_page_size)
    EXPECT_GREATER(size, min_page_size, NULL)
    EXPECT_GREATER(free_space, 0, NULL)
    EXPECT_GREATER(frame_reg, 0, NULL)
    EXPECT_GREATER(free_space_size, 1, NULL)
    EXPECT_NONNULL(manager, NULL);

    page_t *page = malloc(size);
    EXPECT_GOOD_MALLOC(page, NULL);
    page->page_header.page_id = id;
    page->page_header.page_size = size;
    page->page_header.free_space = 0;
    page->page_header.flags.is_dirty  = HAS_FLAG(flags, page_flag_dirty);
    page->page_header.flags.is_fixed  = HAS_FLAG(flags, page_flag_fixed);
    page->page_header.flags.is_locked = HAS_FLAG(flags, page_flag_locked);
    page->frame_register.free_list_len = page->frame_register.in_use_num = page->free_space_register.list_len = 0;
    page->frame_register.max_num_frames = frame_reg;
    page->free_space_register.list_max = free_space;

    lane_reg_init(page, frame_reg);
    freespace_push(page, free_space_begin_offset, free_space_end_offset);

  //  page_hot_store_set(manager, id, page);

    return page;
}

size_t page_get_free_space(
    page_t *page,
    free_space_get_strategy strategy)
{
    EXPECT_NONNULL(page, 0);
    switch (strategy) {
        case free_space_get_quickapprox:
            return page_approx_freespace(page);
        case free_space_get_slowexact:
            return freespace_largest(page);
        default: {
            panic(BADBRANCH, page);
            return 0;
        }
    }
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
 * @return
 */
fid_t *frame_create(
    page_t *page,
    block_pos strategy,
    size_t element_size)
{
    fid_t *handle = NULL;

    EXPECT_NONNULL(page, NULL);
    EXPECT_GREATER(element_size, 0, NULL);

    if (!lane_reg_is_full(page)) {
        handle = malloc (sizeof(lane_reg_t));
        EXPECT_GOOD_MALLOC(handle, NULL);
        if ((handle->frame_id = lane_new(page, strategy, element_size)) == NULL_FID) {
            return NULL;
        }
        handle->page_id = page->page_header.page_id;
    } else error(err_limitreached);

    return handle;
}

static inline bool page_equals(
    page_t  *lhs,
    page_t  *rhs)
{
    return ((lhs != NULL && rhs != NULL) && (lhs->page_header.page_id == rhs->page_header.page_id));
}

zone_t *buf_zone_create(
    anit_buffer_t *manager,
    page_t *frame_page,
    lane_id_t frame_id,
    page_t *new_zone_page,
    block_pos strategy)
{
    zone_t              *retval       = NULL;
    lane_t             *frame        = lane_by_id(frame_page, frame_id);
    const size_t         block_size   = (sizeof(zone_t) + frame->elem_size);
    range_t              free_range;

    if (freespace_bind(&free_range, new_zone_page, block_size, strategy)) {
        zone_t           new_zone            = { .next = PAGE_NULL_PTR() };
        const offset_t   new_zone_offset     = free_range.begin;
        printf("DEBUG: zone offset %zu\n", new_zone_offset);
        page_ptr_type         dist_frame_to_new   = page_equals(frame_page, new_zone_page) ? PAGE_PTR_NEAR : PAGE_PTR_FAR;
        bool             is_first_zone       = ptr_is_null(&frame->first);

        assert (PTR_DISTANCE(free_range.begin, free_range.end) >= block_size);

        if (is_first_zone) {
            assert (ptr_is_null(&frame->last));
            ptr_make(&new_zone.prev, frame_page, dist_frame_to_new, target_frame, PTR_DISTANCE(frame_page, frame));
            ptr_make(&frame->first, new_zone_page, dist_frame_to_new, target_zone, new_zone_offset);
        } else {
            // TODO: Implement partial update in cold store in case one of these pages are not in hot store
            // This step requires to hold max 3 pages in memory (the frame page itself, the page with the last new_zone
            // and (in worst case) an additional page for the last new_zone. To avoid loaded one page from the cold store
            // into the hot store, an alternative can be to apply "partial update" directly in the cold store to
            // update the pointers. With "partial update", no additional page must be loaded from cold store.
            assert (!ptr_is_null(&frame->last));

            zone_t       *last_zone        = ptr_cast_zone(manager, &frame->last);
            page_t       *last_zone_page   = page_anticache_get_page_by_id(manager, frame->last.page_id);
            page_anticache_pin_page(manager, last_zone_page);
            page_ptr_type      dist_last_to_new = page_equals(last_zone_page, new_zone_page) ? PAGE_PTR_NEAR : PAGE_PTR_FAR;

            ptr_make(&new_zone.prev, last_zone_page, dist_last_to_new, target_zone, frame->last.offset);
            ptr_make(&last_zone->next, new_zone_page, dist_last_to_new, target_zone, new_zone_offset);

            assert (ptr_cast_zone(manager, &last_zone->next) != last_zone);

            page_anticache_unpin_page(manager, last_zone_page);
        }

        ptr_make(&frame->last, new_zone_page, dist_frame_to_new, target_zone, new_zone_offset);
        write_unsafe(new_zone_page, new_zone_offset, &new_zone, sizeof(zone_t));
        //memset(read_unsafe(frame_page, new_zone_offset + sizeof(zone_t)), 'X',
        //       frame->elem_size);   // TODO: Remove this line; its just for debugging purposes
        retval = read_unsafe(new_zone_page, new_zone_offset);
    } else {
        error(err_limitreached);
    }

    return retval;
}

zone_t *buf_zone_head(
    anit_buffer_t *manager,
    page_t *page,
    lane_id_t frame_id)
{
    require_non_null(manager);
    require_non_null(page);

    lane_t *frame = lane_by_id(page, frame_id);
    return (ptr_is_null(&frame->first) ? NULL : ptr_cast_zone(manager, &frame->first));
}

zone_t *buf_zone_next(
    anit_buffer_t *manager,
    zone_t *zone)
{
    require_non_null(manager);
    require_non_null(zone);
    if (ptr_is_null(&zone->next)) {
        return NULL;
    } else {
        page_t *page = page_anticache_get_page_by_id(manager, zone->next.page_id);
        zone_t *next_zone = ptr_cast_zone(manager, &zone->next);

        // TODO: REmove this this debug

        printf("DEBUG: this zone offset ? in page id ? (next page id %d, offset %zu)\n",
               zone->next.page_id, zone->next.offset);

        printf("DEBUG: next zone offset %zu in page id %d (next page id %d, offset %zu)\n",
               PTR_DISTANCE(page, next_zone), page->page_header.page_id,
               next_zone->next.page_id, next_zone->next.offset);
        // END Remove

        assert (next_zone != zone);


        return next_zone;
    }
}

bool zone_memcpy(
    zone_t *zone,
    size_t offset,
    const void *data,
    size_t size)
{
    EXPECT_NONNULL(zone, false);
    EXPECT_NONNULL(data, false);
    EXPECT_GREATER(size, 0, false);

    memcpy(zone_get_data(zone) + offset, data, size);
    return true;
}

void *zone_get_data(
    zone_t *zone)
{
    require_non_null(zone);
    return zone + 1;
}

bool zone_remove(
    anit_buffer_t *manager,
    page_t *page,
    const zone_t *zone)
{
    EXPECT_NONNULL(page, false);
    EXPECT_NONNULL(page, zone);
    /*
     *  Case | IN-NULL | OUT-NULL | IN	 | OUT  | Scope	  | Note
     *  -----+---------+----------+------+------+---------+-------------------------
     *  0    | FRAME   | YES	  | NEAR | -    | 1 Page  |
     *  1    | FRAME   | YES	  | FAR	 | -	| 2 Pages | Handled outside this page
     *  2    | FRAME   | NO		  | NEAR | NEAR | 1 Page  |
     *  3    | FRAME   | NO		  | FAR	 | NEAR | 2 Pages | Handled outside this page
     *  4    | FRAME   | NO		  | NEAR | FAR  | 2 Pages | Handled outside this page
     *  5    | FRAME   | NO		  | FAR	 | FAR  | 3 Pages | Handled outside this page
     *  6    | NO	   | YES	  | NEAR | -    | 1 Page  |
     *  7    | NO	   | YES	  | FAR	 | -    | 2 Pages | Handled outside this page
     *  8    | NO	   | NO		  | NEAR | NEAR | 1 Page  |
     *  9    | NO	   | NO		  | FAR	 | NEAR | 2 Pages | Handled outside this page
     *  10   | NO	   | NO		  | NEAR | FAR  | 2 Pages | Handled outside this page
     *  11   | NO	   | NO		  | FAR	 | FAR  | 3 Pages | Handled outside this page
     */

    panic_if(ptr_is_null(&zone->prev), BADBACKLINK, zone);
    panic_if((!ptr_is_null(&zone->next) && !(ptr_typeof(&zone->next) == target_zone)), BADFRWDLINK, zone);
    panic_if(!IN_RANGE(ptr_target, ptr_typeof(&zone->prev), target_frame, target_zone), BADTYPE, &zone->prev);

    bool prev_is_near  = (ptr_has_scope(&zone->prev, type_near_ptr));
    bool next_is_near  = (ptr_has_scope(&zone->next, type_near_ptr));
    bool prev_is_frame = (ptr_typeof(&zone->prev) == target_frame);
    bool prev_is_zone  = (ptr_typeof(&zone->prev) == target_zone);
    bool next_is_null  = (ptr_is_null(&zone->next));

    bool in_page_zone_is_lonely = (prev_is_frame && prev_is_near && next_is_null);
    bool in_page_zone_is_head   = (prev_is_frame && !next_is_null && prev_is_near && next_is_near);
    bool in_page_zone_is_tail   = (next_is_null && prev_is_near && prev_is_zone);
    bool in_page_zone_is_middle = (!next_is_null && prev_is_near && next_is_near && prev_is_zone);

    if (!(in_page_zone_is_lonely | in_page_zone_is_head | in_page_zone_is_tail | in_page_zone_is_middle)) {
        error(err_notincharge);
        return false;
    } else {
        lane_t *frame = lane_by_zone(manager, zone);

        if (in_page_zone_is_lonely) {
        /*
         *  +---------------- THIS PAGE ----------------+       +---------------- THIS PAGE ----------------+
         *  |     v--------- frame -----|               |       |                                           |
         *  |  +-------+            +--------+          |       |  +-------+                                |
         *  |  | FRAME | - first -> | ZONE X | -> NULL  |   =>  |  | FRAME | - first ->   NULL              |
         *  |  +-------+            +--------+          |       |  +-------+                                |
         *  |     |-------- last -------^               |       |     |------- last -------^                |
         *  +-------------------------------------------+       +-------------------------------------------+
         */
            frame->first = PAGE_NULL_PTR();
            frame->last = PAGE_NULL_PTR();
        } else if (in_page_zone_is_head) {
        /*
         *  +---------------- THIS PAGE ----------------------------+       +---------------- THIS PAGE -------------+
         *  |      v--------- frame ----|                           |       |      v--------- frame ----|            |
         *  |  +-------+            +--------+     +------+         |       |  +-------+            +------+         |
         *  |  | FRAME | - first -> | ZONE X | <-> | ZONE | -> ...  |   =>  |  | FRAME | - first -> | ZONE | -> ...  |
         *  |  +-------+            +--------+     +------+         |       |  +-------+            +------+         |
         *  |     |------- last -----------------------^?-------^   |       |     |------- last --------^?-------^   |
         *  +-------------------------------------------------------+       +----------------------------------------+
         */
            zone_t *next_zone = ptr_cast_zone(manager, &zone->next);
            size_t next_zone_offset = PTR_DISTANCE(page, next_zone);

            frame->first = zone->next;
            ptr_make(&next_zone->prev, page, PAGE_PTR_NEAR, target_frame, PTR_DISTANCE(page, frame));
            if (frame->last.offset == next_zone_offset) {
                ptr_make(&frame->last, page, PAGE_PTR_NEAR, target_zone, next_zone_offset);
            }
        } else if (in_page_zone_is_tail) {
        /*
        *  +---------------- THIS PAGE -----------------------------+       +---------------- THIS PAGE -------------+
        *  |      v- frame ----|                                    |       |      v----frame --|                    |
        *  |  +-------+             +------+     +--------+         |       |  +-------+            +------+         |
        *  |  | FRAME | -1st->...-> | ZONE | <-> | ZONE X | -> NULL |   =>  |  | FRAME | -1st->...->| ZONE | -> NULL |
        *  |  +-------+             +------+     +--------+         |       |  +-------+            +------+         |
        *  |     |------- last --------------------^                |       |     |------- last -------^             |
        *  +--------------------------------------------------------+       +----------------------------------------+
        */
            zone_t *prev_zone = ptr_cast_zone(manager, &zone->prev);
            frame->last = zone->prev;
            prev_zone->next = PAGE_NULL_PTR();
        } else if (in_page_zone_is_middle) {
         /*
         *  +---------------- THIS PAGE ----------------------------------------+       +---------------- THIS PAGE --------------------------+
         *  |      v-frame --|                                                  |       |      v-frame --|                                    |
         *  |  +-------+             +------+    +--------+    +------+         |       |  +-------+             +------+    +------+         |
         *  |  | FRAME | -1st->...-> | ZONE | <> | ZONE X | <> | ZONE | -> ...  |   =>  |  | FRAME | -1st->...-> | ZONE | <> | ZONE | -> ...  |
         *  |  +-------+             +------+    +--------+    +------+         |       |  +-------+             +------+    +------+         |
         *  |     |------- last --------------------------------------------^   |       |     |------- last ------------------------------^   |
         *  +-------------------------------------------------------------------+       +-----------------------------------------------------+
         */
            zone_t *prev_zone = ptr_cast_zone(manager, &zone->prev), *next_zone = ptr_cast_zone(manager, &zone->next);
            prev_zone->next = zone->next;
            next_zone->prev = zone->prev;
        } else panic(BADBRANCH, zone);

        freespace_push(page, PTR_DISTANCE(page, zone),
                       PTR_DISTANCE(page, ((void *) zone + sizeof(zone_t) + frame->elem_size)));
        freespace_rebuild(page);
    }

    return true;
}

bool frame_delete(
    fid_t *frame_handle)
{
    panic("Function is not implemented yet: '%s'", "frame_delete");
}

bool page_free(
    page_t *page)
{
    // TODO
    return true;
}

void page_dump(
    FILE *out,
    anit_buffer_t *manager,
    const page_t *page,
    bool hex_view)
{
    assert (out);
    printf("\n#\n");
    printf("# A page dump was requested by the system.\n");
    printf("# \n");
    if (page) {
        size_t free_space_reg_len = freespace(page)->list_max;
        size_t frame_reg_len = lane_reg(page)->max_num_frames;
        size_t total_header_size = page_total_header_sizeof(free_space_reg_len, frame_reg_len);

        freespace_reg_t *free_space_reg = freespace(page);
        lane_reg_t *frame_reg = lane_reg(page);

        size_t page_capacity = (page->page_header.page_size - total_header_size);
        printf("# Page (%p), pid=%d, page_size/capacity/free=%zu/%zu/%zu byte (footprint=%.4f%%, filled=%.4f%%)\n",
               page, page->page_header.page_id,
               page->page_header.page_size,
               page_capacity,
               page->page_header.free_space,
               (page->page_header.page_size - page_capacity)/(float)(page->page_header.page_size) * 100,
               (page_capacity - page->page_header.free_space)/(float)(page_capacity) * 100);
        printf("#\n");
        printf("# Segments:\n");
        printf("# 0x%08x [HEADER]\n", 0);
        printf("# %#010lx  [free space register]\n",     PTR_DISTANCE(page, free_space_reg));
        printf("# %#010lx    capacity: %u\n",            PTR_DISTANCE(page, &free_space_reg->list_max),
                                                            free_space_reg->list_max);
        printf("# %#010lx    page_size: %u\n",                PTR_DISTANCE(page, &free_space_reg->list_len),
                                                            free_space_reg->list_len);
        printf("# %#010lx  [frame register]\n",          PTR_DISTANCE(page, frame_reg));
        printf("# %#010lx    capacity: %u\n",            PTR_DISTANCE(page, &frame_reg->max_num_frames),
                                                            frame_reg->max_num_frames);
        printf("# %#010lx    in-use page_size: %u\n",         PTR_DISTANCE(page, &frame_reg->in_use_num),
                                                            frame_reg->in_use_num);
        printf("# %#010lx    free-list page_size: %u\n",      PTR_DISTANCE(page, &frame_reg->free_list_len),
                                                            frame_reg->free_list_len);
        printf("# %#010lx  [free space data]\n",         PTR_DISTANCE(page, freespace_at(page, 0)));
        for (size_t idx = 0; idx < free_space_reg->list_len; idx++) {
            freespace_at(page, idx);
            range_t *range = freespace_at(page, idx);
            printf("# %#010lx    idx=%zu: off_start=%zu, off_end=%zu\n",
                   PTR_DISTANCE(page, range), idx, range->begin, range->end);
        }
        if (free_space_reg->list_len < free_space_reg->list_max) {
            printf("# %#010lx    (undefined until %#010lx)\n",
                   PTR_DISTANCE(page, freespace_at(page, free_space_reg->list_len)),
                   PTR_DISTANCE(page, freespace_at(page, free_space_reg->list_max - 1)));
        }

        printf("# %#010lx  [frame register in-use list]\n",       PTR_DISTANCE(page, lane_offsetof(page, 0)));
        for (lane_id_t frame_id = 0; frame_id < frame_reg_len; frame_id++) {
            offset_t *offset = lane_offsetof(page, frame_id);
            printf("# %#010lx    fid=%05u: offset=", PTR_DISTANCE(page, offset), frame_id);
            if (*offset != NULL_OFFSET) {
                printf("%#010lx\n", *offset);
            } else
                printf("(unset)\n");
        }

        printf("# %#010lx  [frame register free-list stack]\n",       PTR_DISTANCE(page, lane_reg_entry_by_pos(page, 0)));
        for (size_t idx = 0; idx < frame_reg->free_list_len; idx++) {
            lane_id_t *frame = lane_reg_entry_by_pos(page, idx);
            printf("# %#010lx    pos=%05zu: fid=%u\n", PTR_DISTANCE(page, frame), idx, *frame);
        }

        for (size_t idx = frame_reg->free_list_len; idx < frame_reg->max_num_frames; idx++) {
            lane_id_t *frame = lane_reg_entry_by_pos(page, idx);
            printf("# %#010lx    pos=%05zu: (unset)\n", PTR_DISTANCE(page, frame), idx);
        }

        printf("# %#010lx [PAYLOAD]\n",                   PTR_DISTANCE(page, seek(page, SEEK_PAYLOAD)));

        printf("# ---------- [FRAMES IN USE]\n");
        for (lane_id_t frame_id = lane_reg_scan(page, frame_inuse); frame_id != NULL_FID;
             frame_id = lane_reg_scan(NULL, frame_inuse))
        {
            lane_t *frame = lane_by_id(page, frame_id);
            assert (frame);

            offset_t frame_offset = *lane_offsetof(page, frame_id);
            printf("# %#010lx    elem_size:%zu, first:(far_ptr:%d, pid=%d, offset=%#010lx), "
                   "last:(far_ptr:%d, pid=%d, offset=%#010lx) \n",
                       frame_offset, frame->elem_size, frame->first.is_far_ptr,
                       frame->first.page_id, frame->first.offset, frame->last.is_far_ptr,
                       frame->last.page_id, frame->last.offset);
        }

        printf("# ---------- [ZONES]\n");
        for (lane_id_t frame_id = lane_reg_scan(page, frame_inuse); frame_id != NULL_FID;
             frame_id = lane_reg_scan(NULL, frame_inuse)) {

            lane_t *frame = lane_by_id(page, frame_id);
            in_page_ptr ptr = lane_by_id(page, frame_id)->first;
            while (!ptr_is_null(&ptr) && ptr_has_scope(&ptr, type_near_ptr)) {
                assert(ptr.page_id == page->page_header.page_id);

                const zone_t *zone = ptr_cast_zone(manager, &ptr);

                printf("# %#010lx    prev:(far_ptr:%d, pid=%d, offset=%#010lx), next:(far_ptr:%d, pid=%d, offset=%#010lx)\n",
                       PTR_DISTANCE(page, zone), zone->prev.is_far_ptr, zone->prev.page_id, zone->prev.offset,
                        zone->next.is_far_ptr, zone->next.page_id, zone->next.offset);

                if (hex_view) {
                    printf("# zone content  ");
                    for (unsigned i = 0; i < 16; i++) {
                        printf("%02x ", i);
                    }
                    printf("\n");

                    const void *data = (zone + 1);
                    char *puffer = malloc(16);
                    size_t block_end = 16;

                    for (; block_end < frame->elem_size; block_end += 16) {
                        memset(puffer, 0, 16);
                        memcpy(puffer, data + block_end - 16, 16);
                        printf("# %#010lx    ", PTR_DISTANCE(page, data + block_end));
                        for (unsigned i = 0; i < 16; i++) {
                            printf("%02X ", (unsigned char) puffer[i]);
                        }
                        for (unsigned i = 0; i < 16; i++) {
                            printf("%c ", puffer[i]);
                        }
                        printf("\n");
                    }

                    unsigned bytes_remain = 16 - (block_end - frame->elem_size);
                    if (bytes_remain > 0) {
                        memset(puffer, 0, 16);
                        memcpy(puffer, data + (block_end - 16), bytes_remain);
                        printf("# %#010lx    ", PTR_DISTANCE(page, data + block_end + 1));
                        for (unsigned i = 0; i < bytes_remain; i++) {
                            printf("%02X ", (unsigned char) puffer[i]);
                        }
                        for (unsigned i = bytes_remain; i < 16; i++) {
                            printf("   ");
                        }

                        for (unsigned i = 0; i < bytes_remain; i++) {
                            printf("%c ", puffer[i]);
                        }
                        for (unsigned i = bytes_remain; i < 16; i++) {
                            printf("   ");
                        }

                    }
                    printf("\n");

                    free(puffer);
                }




                ptr = zone->next;
            }
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

static inline size_t page_ext_header_sizeof(
        size_t free_space_cap,
        size_t frame_reg_cap)
{
    return (free_space_cap * sizeof(range_t) + frame_reg_cap * sizeof(offset_t) + frame_reg_cap * sizeof(lane_id_t));
}

static inline size_t page_total_header_sizeof(
        size_t free_space_cap,
        size_t frame_reg_cap)
{
    return CORE_HDR_SIZE + page_ext_header_sizeof(free_space_cap, frame_reg_cap);
}

static inline void *seek(
    const page_t *page,
    seek_target target)
{
    switch (target) {
        case SEEK_FREESPACE_REGISTER:
            return (void *) page + sizeof(page_header_t);
        case SEEK_LANE_REGISTER:
            return seek(page, SEEK_FREESPACE_REGISTER) + sizeof(freespace_reg_t);
        case SEEK_FREESPACE_ENTRIES:
            return seek(page, SEEK_LANE_REGISTER) + sizeof(lane_reg_t);
        case SEEK_LANE_INUSE:
            return seek(page, SEEK_FREESPACE_ENTRIES) + sizeof(range_t) * freespace(page)->list_max;
        case SEEK_LANE_FREELIST:
            return seek(page, SEEK_LANE_INUSE) + lane_reg(page)->max_num_frames * sizeof(offset_t);
        case SEEK_PAYLOAD:
            return seek(page, SEEK_LANE_FREELIST) +
                    lane_reg(page)->max_num_frames * sizeof(lane_id_t);
        default: panic("Unknown seek target '%d'", target);
    }
}

static inline freespace_reg_t *freespace(
        const page_t *page)
{
    return (freespace_reg_t *) seek(page, SEEK_FREESPACE_REGISTER);
}

static inline void lane_reg_init(
        page_t *page,
        size_t num_frames)
{
    lane_reg_t *frame_reg = lane_reg(page);

    frame_reg->free_list_len = frame_reg->max_num_frames = num_frames;
    for (size_t idx = 0; idx < frame_reg->free_list_len; idx++) {
        *lane_reg_entry_by_pos(page, idx) = frame_reg->free_list_len - idx - 1;
    }

    frame_reg->in_use_num = 0;
    for (size_t idx = 0; idx < frame_reg->max_num_frames; idx++) {
        *lane_offsetof(page, idx) = NULL_OFFSET;
    }
}

static inline lane_reg_t *lane_reg(
        const page_t *page)
{
    return (lane_reg_t *) seek(page, SEEK_LANE_REGISTER);
}

static inline range_t *freespace_at(
        const page_t *page,
        size_t pos)
{
    freespace_reg_t *free_space_reg = freespace(page);
    assert (pos < free_space_reg->list_max);
    return seek(page, SEEK_FREESPACE_ENTRIES) + sizeof(range_t) * pos;
}

static inline size_t freespace_len(
        const page_t *page)
{
    freespace_reg_t *free_space_reg = freespace(page);
    return free_space_reg->list_len;
}

static inline bool freespace_pop(
        range_t *range,
        page_t *page)
{
    freespace_reg_t *free_space_reg = freespace(page);
    size_t len = freespace_len(page);
    if (len > 0) {
        if (range != NULL) {
            *range = *freespace_at(page, freespace_len(page) - 1);
        }
        free_space_reg->list_len--;
        return true;
    } else return false;
}

static inline offset_t *lane_offsetof(
        const page_t *page,
        lane_id_t frame_id)
{
    assert (page);
    lane_reg_t *frame_reg = lane_reg(page);
    assert (frame_id < frame_reg->max_num_frames);
    return seek(page, SEEK_LANE_INUSE) + frame_id * sizeof(offset_t);
}

static inline lane_t *lane_by_id(
        const page_t *page,
        lane_id_t frame_id)
{
    assert (page);
    offset_t offset = *lane_offsetof(page, frame_id);
    void *data = read_unsafe(page, offset);
    EXPECT_NONNULL(data, NULL);
    return data;
}

static inline lane_id_t *lane_reg_entry_by_pos(
        const page_t *page,
        size_t pos)
{
    lane_reg_t *frame_reg = lane_reg(page);
    assert (pos < frame_reg->max_num_frames);
    return seek(page, SEEK_LANE_INUSE) + frame_reg->max_num_frames * sizeof(offset_t) +
            pos * sizeof(lane_id_t);
}

static inline bool lane_reg_is_full(
        const page_t *page)
{
    lane_reg_t *frame_reg = lane_reg(page);
    return (frame_reg->free_list_len == 0);
}

static inline lane_id_t lane_new(
        page_t *page,
        block_pos strategy,
        size_t size)
{
    assert (!lane_reg_is_full(page));

    lane_id_t frame_id = NULL_FID;
    range_t free_range;
    if ((freespace_bind(&free_range, page, FRAME_HDR_SIZE, strategy))) {
        frame_id = *lane_reg_entry_by_pos(page, --(lane_reg(page))->free_list_len);

        lane_t frame = {
            .first = PAGE_NULL_PTR(),
            .last = PAGE_NULL_PTR(),
            .elem_size = size
        };
        offset_t frame_offset = free_range.begin;
        assert (PTR_DISTANCE(free_range.begin, free_range.end) >= sizeof(lane_t));
        write_unsafe(page, free_range.begin, &frame, sizeof(lane_t));
        lane_reg_link(page, frame_id, frame_offset);
    }
    return frame_id;
}

static inline void lane_reg_link(
        page_t *page,
        lane_id_t frame_id,
        offset_t frame_offset)
{
    assert(page);
    lane_reg_t *frame_reg = lane_reg(page);
    assert(frame_id < frame_reg->max_num_frames);
    *(offset_t *)(seek(page, SEEK_LANE_INUSE) + frame_id * sizeof(offset_t)) = frame_offset;
}

static inline range_t *freespace_new(
        const page_t *page)
{
    range_t *entry = NULL;
    freespace_reg_t *free_space_stack = freespace(page);
    if (free_space_stack->list_len < free_space_stack->list_max) {
        size_t entry_pos = free_space_stack->list_len++;
        entry = freespace_at(page, entry_pos);
    }
    return entry;
}

static inline size_t freespace_find_first(
        const page_t *page,
        size_t capacity)
{
    assert (page);
    assert (capacity > 0);

    size_t entry_id_cursor = freespace_len(page);

    while (entry_id_cursor--) {
        range_t *cursor = freespace_at(page, entry_id_cursor);
        if (OFFSET_DISTANCE(cursor->begin, cursor->end) >= capacity) {
            goto return_position;
        }
    }

    entry_id_cursor = SIZE_MAX;
return_position:

    return entry_id_cursor;
}

static inline size_t freespace_largest(const page_t *page)
{
    assert (page);

    size_t max_capacity = 0;
    size_t entry_id_cursor = freespace_len(page);

    while (entry_id_cursor--) {
        range_t *cursor = freespace_at(page, entry_id_cursor);
        max_capacity = max(max_capacity, OFFSET_DISTANCE(cursor->begin, cursor->end));
    }

    return max_capacity;
}

static inline bool freespace_bind(
        range_t *range,
        page_t *page,
        size_t size,
        block_pos strategy)
{
    assert(page);
    assert(size > 0);
    assert(range);

    range->begin = MAX_OFFSET;
    range->end = NULL_OFFSET;

    const size_t no_such_entry_id = freespace_len(page);
    size_t entry_id_cursor = no_such_entry_id, best_entry_id = no_such_entry_id;

    switch (strategy) {
        case positioning_first_nomerge: case positioning_first_merge:
            /*while (entry_id_cursor--) {
                range_t *cursor = freespace_at(page, entry_id_cursor);
                if (OFFSET_DISTANCE(cursor->begin, cursor->end) >= page_size) {
                    *range = freespace_split(page, entry_id_cursor, page_size);
                    break;
                }
            }*/
            entry_id_cursor = freespace_find_first(page, size);
            if (entry_id_cursor != SIZE_MAX)
                *range = freespace_split(page, entry_id_cursor, size);

            break;
        case positioning_smallest_nomerge: case positioning_smallest_merge:
            while (entry_id_cursor--) {
                range_t *cursor = freespace_at(page, entry_id_cursor);
                size_t cursor_block_size = OFFSET_DISTANCE(cursor->begin, cursor->end);
                if (cursor_block_size >= size &&
                    cursor_block_size < OFFSET_DISTANCE(range->begin, range->begin)) {
                    range->begin = cursor->begin;
                    range->end = cursor->end;
                    best_entry_id = entry_id_cursor;
                }
            }
            *range = freespace_split(page, best_entry_id, size);
            break;
        case positioning_largest_nomerge: case positioning_largest_merge:
            while (entry_id_cursor--) {
                range_t *cursor = freespace_at(page, entry_id_cursor);
                size_t cursor_block_size = OFFSET_DISTANCE(cursor->begin, cursor->end);
                if (cursor_block_size >= size &&
                    cursor_block_size > OFFSET_DISTANCE(range->begin, range->begin)) {
                    range->begin = cursor->begin;
                    range->end = cursor->end;
                    best_entry_id = entry_id_cursor;
                }
            }
            *range = freespace_split(page, best_entry_id, size);
            break;
        default:
            error(err_internal);
            break;
    }

    if (entry_id_cursor != no_such_entry_id) {
        if ((strategy == positioning_first_merge) ||
            (strategy == positioning_smallest_merge) ||
            (strategy == positioning_largest_merge)) {
            freespace_rebuild(page);
        }
    }

    bool success = (range->begin < range->end);
    if (!success) {
        error(err_no_free_space);
    }

    return success;
}

static inline range_t freespace_split(
        page_t *page,
        size_t pos,
        size_t size)
{
    assert (page);
    freespace_reg_t *free_space_reg = freespace(page);
    assert (pos < free_space_reg->list_len);
    range_t *range = freespace_at(page, pos);
    assert (size <= PTR_DISTANCE(range->begin, range->end));
    range_t result = {
        .begin = range->begin,
        .end = range->begin + size
    };
    range->begin = result.end;
    page_approx_freespace_dec(page, size);
    return result;
}

static inline void freespace_rebuild(
        page_t *page)
{
    assert (page);
    freespace_cleanup(page);
    freespace_merge(page);
}

static inline void freespace_cleanup(
        page_t *page)
{
    assert (page);
    size_t idx = freespace_len(page);
    while (idx--) {
        range_t *range = freespace_at(page, idx);
        assert (range->begin <= range->end);
        if (OFFSET_DISTANCE(range->begin, range->end) == 0) {
            size_t reg_len = freespace_len(page);
            if (idx < reg_len) {
                memcpy((void *) range, freespace_at(page, idx) + 1, (reg_len - idx) * sizeof(range_t));
            }
            bool success = freespace_pop(NULL, page);
            assert(success);
            idx++;
        }
    }
}

static inline int freespace_comp_by_start(
        const void *lhs,
        const void *rhs)
{
    offset_t a = ((range_t *) lhs)->begin;
    offset_t b = ((range_t *) rhs)->begin;
    return (a == b ? 0 : (a < b) ? - 1 : 1);
}

static inline void freespace_merge(
        page_t *page)
{
    assert (page);
    freespace_reg_t *free_space_reg = freespace(page);
    size_t len = freespace_len(page);
    vector_t *vector = vector_create(sizeof(range_t), len);

    size_t idx = len;
    while (idx--) {
        range_t *range = freespace_at(page, idx);
        vector_add(vector, 1, range);
    }

    void *raw_data = vector_get(vector);
    qsort(raw_data, len, sizeof(range_t), freespace_comp_by_start);

    vector_t *stack = vector_create(sizeof(range_t), len);
    vector_add(stack, 1, raw_data); // ... stack is buggy

    for (size_t range_idx = 0; range_idx < len; range_idx++) {
        range_t *current_range = (range_t *)(raw_data + range_idx * sizeof(range_t));
        range_t *stack_top = vector_peek(stack);
        if (range_do_overlap(current_range, stack_top)) {
            stack_top->end = max(stack_top->end, current_range->end);
        } else vector_add(stack, 1, current_range);
    }

    free_space_reg->list_len = stack->num_elements;
    memcpy(freespace_at(page, 0), vector_get(stack), stack->num_elements * sizeof(range_t));

    vector_free(vector);
    vector_free(stack);
}

static inline lane_id_t lane_reg_scan(
        const page_t *page,
        frame_state state)
{
    static size_t frame_id;
    static page_t *last_page = NULL;
    if (page != NULL) {
        last_page = (page_t *) page;
        frame_id = lane_reg(last_page)->max_num_frames;
    }

    while (frame_id--) {
        offset_t frame_offset = *lane_offsetof(last_page, frame_id);
        if ((state == frame_inuse && frame_offset != NULL_OFFSET) ||
            (state == frame_free  && frame_offset == NULL_OFFSET))
            return frame_id;
    }

    return NULL_FID;
}

static inline bool ptr_is_null(
        const in_page_ptr *ptr)
{
    assert(ptr);
    return (ptr->offset == NULL_OFFSET);
}

/*static inline bool in_page_ptr_equals(
        const in_page_ptr *lhs,
        const in_page_ptr *rhs)
{
    assert(lhs);
    assert(rhs);
    return (lhs->page_id == rhs->page_id) &&
            (lhs->is_far_ptr == rhs->is_far_ptr) &&
            (lhs->offset == rhs->offset) &&
            (lhs->target_type_bit_0 == rhs->target_type_bit_0) &&
            (lhs->target_type_bit_1 == rhs->target_type_bit_1);
}
*/

static inline bool ptr_has_scope(
        const in_page_ptr *ptr,
        ptr_scope_type type)
{
    assert (ptr);
    switch (type) {
        case type_far_ptr: return (ptr->is_far_ptr);
        case type_near_ptr: return (!ptr->is_far_ptr);
        default:
            error(err_internal);
            return false;
    }
}

static inline void ptr_make(
        in_page_ptr *ptr,
        page_t *page,
        page_ptr_type type,
        ptr_target target,
        offset_t offset)
{
    assert(ptr);
    assert(page);
    assert(offset != NULL_OFFSET);
    ptr->page_id = page->page_header.page_id;
    ptr->is_far_ptr = (type == PAGE_PTR_FAR);
    ptr->offset = offset;
    switch (target) {
        case target_frame:
            ptr->target_type_bit_0 = ptr->target_type_bit_1 = 0;
            break;
        case target_zone:
            ptr->target_type_bit_0 = 0;
            ptr->target_type_bit_1 = 1;
            break;
        case target_userdata:
            ptr->target_type_bit_0 = 1;
            ptr->target_type_bit_1 = 0;
            break;
        case target_corrupted:
            ptr->target_type_bit_0 = 1;
            ptr->target_type_bit_1 = 1;
            break;
        default:
            panic("Unknown pointer target type '%d'", target);
    }
}

static inline ptr_target ptr_typeof(
        const in_page_ptr *ptr)
{
    assert (ptr);
    if (ptr->target_type_bit_0 == 0) {
        if (ptr->target_type_bit_1 == 0) {
            return target_frame;
        } else {
            return target_zone;
        }
    } else {
        if (ptr->target_type_bit_1 == 0) {
            return target_userdata;
        } else {
            return target_corrupted;
        }
    }
}

static inline void *ptr_deref(
        anit_buffer_t *buffer_manager,
        const in_page_ptr *ptr)
{
    panic_if((buffer_manager == NULL), BADARGNULL, to_string(buffer_manager));
    panic_if((ptr == NULL), BADARGNULL, to_string(ptr));
    panic_if((buffer_manager->page_anticache.page_hot_store == NULL), UNEXPECTED, "page register hash table is null");
    void *page_base_ptr = page_hot_store_fetch(buffer_manager, ptr->page_id);
    panic_if((page_base_ptr == NULL), UNEXPECTED, "page base pointer is not allowed to be null");
    return (page_base_ptr + ptr->offset);
}

static inline zone_t *ptr_cast_zone(
    anit_buffer_t *buffer_manager,
    const in_page_ptr *ptr)
{
    panic_if(ptr_typeof(ptr) != target_zone, BADCAST, ptr);
    void *data = ptr_deref(buffer_manager, ptr);
    return (zone_t *) data;
}

static inline lane_t *ptr_cast_frame(
    anit_buffer_t *buffer_manager,
    const in_page_ptr *ptr)
{
    panic_if(ptr_typeof(ptr) != target_frame, BADCAST, ptr);
    void *data = ptr_deref(buffer_manager, ptr);
    return (lane_t *) data;
}

static inline lane_t *lane_by_zone(
        anit_buffer_t *manager,
        const zone_t *zone)
{
    while (!ptr_is_null(&zone->prev) && ptr_typeof(&zone->prev) != target_frame) {
        zone = ptr_cast_zone(manager, &zone->prev);
        panic_if(!ptr_has_scope(&zone->prev, type_near_ptr), BADHURRY, "Traversing over page boundaries "
                "not implemented!");
    }
    return ptr_cast_frame(manager, &zone->prev);
}

static inline void write_unsafe(
        page_t *page,
        offset_t offset,
        const void *data,
        size_t size)
{
    assert (page);
    assert (offset >= PTR_DISTANCE(page, seek(page, SEEK_PAYLOAD)));
    assert (data);
    assert (size > 0);

    void *base = (void *) page;
    memcpy(base + offset, data, size);
}

static inline void *read_unsafe(
        const page_t *page,
        offset_t offset)
{
    assert (page);
    assert (offset >= PTR_DISTANCE(page, seek(page, SEEK_PAYLOAD)));
    return offset != NULL_OFFSET ? (void *) page + offset : NULL;
}

static inline bool range_do_overlap(
    range_t *lhs,
    range_t *rhs)
{
    return (lhs->end >= rhs->begin && lhs->begin <= rhs->begin) ||
           (rhs->end >= lhs->begin && rhs->begin <= lhs->begin);
}

static inline void page_approx_freespace_inc(
        page_t *page,
        size_t size)
{
    assert (page);
    page->page_header.free_space += size;
}

static inline void page_approx_freespace_dec(
        page_t *page,
        size_t size)
{
    assert (page);
    page->page_header.free_space -= size;
}

static inline size_t page_approx_freespace(
        const page_t *page)
{
    return page->page_header.free_space;
}

static inline bool freespace_push(
        page_t *page,
        offset_t begin,
        offset_t end)
{
    range_t *entry = NULL;
    if ((entry = freespace_new(page))) {
        EXPECT_NONNULL(entry, false);
        entry->begin = begin;
        entry->end = end;
    }
    panic_if((entry == NULL), UNEXPECTED, "Free store capacity exceeded");
    page_approx_freespace_inc(page, OFFSET_DISTANCE(begin, end));
    return (entry != NULL);
}