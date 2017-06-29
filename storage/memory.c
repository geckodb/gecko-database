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
#include <macros.h>
#include <msg.h>
#include <require.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define INIT_NUM_PAGES  1024
#define NULL_FID        UINT_MAX
#define MAX_OFFSET      SIZE_MAX
#define NULL_PAGE_ID    UINT_MAX
#define NULL_OFFSET     0
#define CORE_HDR_SIZE   (sizeof(page_header_t) + sizeof(free_store_t) + sizeof(frame_store_t))
#define FRAME_HDR_SIZE  (sizeof(frame_t))
#define MIN_DATA_SIZE   (sizeof(zone_t) + sizeof(in_page_ptr))

#define MSG_BADBRANCH    "Internal error: unknown condition during operation on object %p."
#define MSG_BADCAST      "Unsupported cast request for persistent ptr %p."
#define MSG_BADTYPE      "Persistent ptr %p points to illegal type."
#define MSG_BADBACKLINK  "Internal error: back link for persistent ptr %p is not allowed to be null."
#define MSG_BADFRWDLINK  "Internal error: forward link for persitent ptr %p is not allowed to point to a non-zone obj."
#define BADPOOLINIT      "Internal error: page pool %p is already initialized."
#define BADENTRYINIT     "Internal error: bad page pool init request from %p to %p"
#define BADPAGEID        "Internal error: page id '%du' was not loaded into hot store. Maybe it does not exists."
#define BADARGNULL       "Illegal argument for function. Parameter '%s' points to null"
#define BADARGZERO       "Illegal argument for function. Parameter '%s' is zero"
#define UNEXPECTED       "Unexpected behavior: '%s'"
#define BADHURRY         "Not implemented: '%s'"
#define BADPAGESIZE      "Request to create a page size of %zuB with a hot store limit of %zuB is illegal"
#define BADSTATE         "Bad state: %s"
#define BADRESULT        "Return value of '%s' is unexpected"
#define BADHOTSTOREINIT  "Memory allocation failed: unable to initialize hot store of buffer manager"
#define BADCOLDSTOREINIT "Memory allocation failed: unable to initialize cold store of buffer manager"
#define BADFREELISTINIT  "Memory allocation failed: unable to initialize page free list in buffer manager"
#define BADHOTSTOREOBJ   "Page with id '%du' is not located in hot store"
#define BADINTERNAL      "Internal error: %s"


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

typedef enum {
    ptr_type_near,
    ptr_type_far
} ptr_type;

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define output_error(code)                                                                                             \
    {                                                                                                                  \
        error(code);                                                                                                   \
        error_print();                                                                                                 \
        fprintf(stderr, "\t> occurred here: '%s:%d'\n", __FILE__, __LINE__);                                           \
        fflush(stderr);                                                                                                \
        abort();                                                                                                       \
    }

#define return_if(expr, error_code, retval)                                                                            \
    if (expr) {                                                                                                        \
        output_error(error_code);                                                                                      \
        return retval;                                                                                                 \
    }

#define debug_var(type, name, value)                                                                                   \
    type name = value;

#define num_args(...)                                                                                                  \
    (sizeof((int[]){__VA_ARGS__})/sizeof(int))

#define in_range(type, val, ...)                                                                                       \
    ({                                                                                                                 \
        bool retval = false;                                                                                           \
        type *list = (type[]){__VA_ARGS__};                                                                            \
        for (unsigned i = 0; i < num_args(__VA_ARGS__); i++)                                                           \
            if (*(list + i) == val) {                                                                                  \
                retval = true;                                                                                         \
                break;                                                                                                 \
            }                                                                                                          \
        retval;                                                                                                        \
    })

#define expect_non_null(obj, retval)              return_if((obj == NULL), err_null_ptr, retval)
#define expect_equals(obj, val, retval)           return_if((obj != val), err_illegal_args, retval)
#define expect_greater(val, lower_bound, retval)  return_if((val <= lower_bound), err_illegal_args, retval)
#define expect_less(val, upper_bound, retval)     return_if((val >= upper_bound), err_illegal_args, retval)
#define expect_good_malloc(obj, retval)           return_if((obj == NULL), err_bad_malloc, retval)

#define ptr_distance(a, b)                                                                                             \
    ((void *)b > (void *)a ? ((void *)b - (void *)a) : ((void *)a - (void *)b))

#define offset_distance(a, b)                                                                                          \
    (a < b ? b - a : NULL_OFFSET )

#define has_flag(val, flag)                                                                                            \
    ((val & flag) == flag)

#define null_ptr()                                                                                                     \
    (in_page_ptr) {                                                                                                 \
        .offset = NULL_OFFSET,                                                                                         \
        .target_type_bit_0 = 1,                                                                                        \
        .target_type_bit_1 = 1,                                                                                        \
        .is_far_ptr = false,                                                                                           \
        .page_id = 0                                                                                                   \
    }

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
static inline void free_store_free_space_inc(page_t *page, size_t size);
static inline void free_store_free_space_dec(page_t *page, size_t size);
static inline size_t free_store_free_space_get(const page_t *page);
static inline size_t free_store_find_range_pos_with_capacity(const page_t *page, size_t capacity);
static inline size_t free_store_get_max_free_range_capacity(const page_t *page);

static inline bool in_page_ptr_is_null(const in_page_ptr *ptr);
static inline bool in_page_ptr_has_scope(const in_page_ptr *ptr, ptr_scope_type type);
static inline void in_page_ptr_make(in_page_ptr *ptr, page_t *page, ptr_type type, ptr_target target, offset_t offset);
static inline void in_page_ptr_make_near(in_page_ptr *ptr, page_t *page, ptr_target type, offset_t offset);
static inline ptr_target in_page_ptr_get_type(const in_page_ptr *ptr);
static inline void *in_page_ptr_deref(buffer_manager_t *buffer_manager, const in_page_ptr *ptr);
static inline zone_t *ptr_cast_zone(buffer_manager_t *buffer_manager, const in_page_ptr *ptr);
static inline frame_t *ptr_cast_frame(buffer_manager_t *buffer_manager, const in_page_ptr *ptr);
static inline frame_t *find_frame(buffer_manager_t *manager, const zone_t *zone);
//static inline void *persistent_ptr_cast_userdata(const page_t *page, in_page_ptr *ptr);


static inline int comp_range_start(const void *lhs, const void *rhs);

static inline void frame_store_init(page_t *page, size_t num_frames);
static inline frame_store_t *frame_store_in(const page_t *page);
static inline offset_t *frame_store_offset_of(const page_t *page, frame_id_t frame_id);
static inline frame_t *frame_store_frame_by_id(const page_t *page, frame_id_t frame_id);
static inline frame_id_t *frame_store_recycle(const page_t *page, size_t pos);
static inline bool frame_store_is_full(const page_t *page);
static inline frame_id_t frame_store_create(page_t *page, block_positioning strategy, size_t size);
static inline void frame_store_link(page_t *page, frame_id_t frame_id, offset_t frame_offset);
static inline frame_id_t frame_store_scan(const page_t *page, frame_state state);

static inline void data_store_write(page_t *page, offset_t offset, const void *data, size_t size);
static inline void *data_store_at_unsafe(const page_t *page, offset_t offset);

static inline bool range_do_overlap(range_t *lhs, range_t *rhs);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

buffer_manager_t *buffer_manager_create(
    size_t page_size,
    size_t free_space_reg_capacity,
    size_t frame_reg_capacity,
    size_t max_hot_store_size)
{
    expect_greater(page_size, 0, NULL);
    expect_greater(max_hot_store_size, 0, NULL);
    expect_greater(free_space_reg_capacity, 0, NULL);
    expect_greater(frame_reg_capacity, 0, NULL);

    panic_if((max_hot_store_size < 2 * page_size), BADPAGESIZE, page_size, max_hot_store_size);

    buffer_manager_t *result = malloc(sizeof(buffer_manager_t));
    expect_good_malloc(result, NULL);
    result->page_hot_store = fixed_linear_hash_table_create(&(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
                                                           sizeof(page_id_t), sizeof(void*), HOTSTORE_INITCAP,
                                                           HOTSTORE_GROW_FACTOR, HOTSTORE_MAX_FILL_FAC);
    result->max_size_hot_store = max_hot_store_size;
    expect_non_null(result->page_hot_store, NULL);

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
    buffer_manager_t    *manager,
    size_t               size,
    page_id_t           (*iterate)(buffer_manager_t *, page_id_t),
    page_t *            (*fetch)(buffer_manager_t *, page_id_t),
    size_t              (*get_capacity)(page_t *, free_space_get_strategy)
)
{
    page_id_t cursor = NULL_PAGE_ID;
    page_t *page = NULL;

    while ((cursor = iterate(manager, cursor)) != NULL_PAGE_ID) {
        page = fetch(manager, cursor);
        if ((get_capacity(page, free_space_get_quickapprox) >= size) &&
            (get_capacity(page, free_space_get_slowexact) >= size)) {
            return page;
        }
    }

    return page;
}

static inline page_t *page_anticache_find_page_by_free_size(
    buffer_manager_t    *manager,
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
        } else if (page == NULL && !page_anticache_cold_store_is_empty(manager)) {
            page = generic_store_get(manager, size, page_anticache_cold_store_iterate, page_cold_store_fetch, page_get_free_space);
            page_anticache_hot_store_add(manager, page);
        } else {
            page = page_anticache_create_page(manager);
            page_anticache_hot_store_add(manager, page);
        }
    }

    return page;
}

static inline void page_anticache_pin_page(
    buffer_manager_t    *manager,
    page_t              *page)
{
    warn(NOTIMPLEMENTED, to_string(page_anticache_pin_page));   // TODO: Implement
    printf("DEBUG: request was for page id %d\n", page->page_header.page_id);
}

static inline void page_anticache_unpin_page(
    buffer_manager_t    *manager,
    page_t              *page)
{
    warn(NOTIMPLEMENTED, to_string(page_anticache_unpin_page));   // TODO: Implement
}

static inline frame_id_t page_anticache_create_frame(
    buffer_manager_t    *manager,
    page_t              *page,
    block_positioning    strategy,
    size_t               size
) {
    require_non_null(manager);
    require_non_null(page);
    require((size > 0), "size must be non-zero");
    panic_if(!(page_hot_store_has(manager, page->page_header.page_id)), BADHOTSTOREOBJ, page->page_header.page_id);
    fid_t *fid = frame_create(page, strategy, size);            // TODO: just return frame_id here instead of frame handle
    panic_if((fid == NULL), UNEXPECTED, "Frame handle is null");
    frame_id_t result = fid->frame_id;
    free (fid);
    return result;
}

block_ptr *buffer_manager_block_alloc(
    buffer_manager_t    *manager,
    size_t size, size_t  nzones,
    block_positioning    strategy)
{
    expect_non_null(manager, NULL);
    expect_greater(size, 0, NULL);
    expect_greater(nzones, 0, NULL);

    block_ptr *result = require_good_malloc(sizeof(block_ptr));
    page_t *page_frame = page_anticache_find_page_by_free_size(manager, FRAME_HDR_SIZE, NULL);
    frame_id_t frame = page_anticache_create_frame(manager, page_frame, strategy, size);

    page_anticache_pin_page(manager, page_frame);

    while (nzones--) {
        page_t *page_zone = page_anticache_find_page_by_free_size(manager, size, page_frame);
        zone_t *new_zone = NULL;

        page_anticache_pin_page(manager, page_zone);
        nzones += ((new_zone = buf_zone_create(manager, page_frame, frame, page_zone, strategy)) == NULL);
        page_anticache_unpin_page(manager, page_zone);
    }

    page_anticache_unpin_page(manager, page_frame);

    *result = (block_ptr) {
        .page_id   = page_frame->page_header.page_id,
        .frame_id  = frame,
        .manager   = manager,
        .zone      = NULL,
        .state     = block_state_closed,
    };

    return result;
}

void buffer_manager_block_free(
    block_ptr *ptr)
{
    require_non_null(ptr);
    assert ((ptr->state != block_state_opened) || (ptr->zone != NULL));
    free (ptr);
}

zone_id_t buffer_manager_block_nextzone(
    block_ptr *ptr)
{
    panic(NOTIMPLEMENTED, to_string(buffer_manager_block_nextzone));   // TODO: Implement
    return 0;
}

void buffer_manager_block_setzones(
    block_ptr *ptr,
    zone_id_t last_zone)
{
    panic(NOTIMPLEMENTED, to_string(buffer_manager_block_setzones));   // TODO: Implement
}

void buffer_manager_block_rmzone(
    block_ptr *ptr,
    zone_id_t zone)
{
    panic(NOTIMPLEMENTED, to_string(buffer_manager_block_rmzone));   // TODO: Implement
}

void buffer_manager_block_open(
    block_ptr *ptr)
{
    require_non_null(ptr);
    require_non_null(ptr->manager);
    panic_if((ptr->state == block_state_opened), BADSTATE, "block was already opened.")

    page_anticache_activate_page_by_id(ptr->manager, ptr->page_id);
    ptr->state = block_state_opened;
}

bool buffer_manager_block_next(
    block_ptr *ptr)
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
        return true;
    } else return false;
}

void buffer_manager_block_close(
    block_ptr *ptr)
{
    require_non_null(ptr);
    ptr->state = block_state_closed;
}

void buffer_manager_zone_read(
    block_ptr *ptr,
    void *capture,
    void (*consumer) (void *capture, const void *data))
{
    require_non_null(ptr);
    require_non_null(consumer);
    panic_if((ptr->zone == NULL), BADSTATE, "read operation on illegal zone");
    consumer (capture, zone_get_data(ptr->zone));
}

void buffer_manager_zone_cpy(
    block_ptr *dst,
    size_t offset,
    const void *src, size_t num)
{
    require_non_null(dst);
    require_non_null(src);
    panic_if((dst->state != block_state_opened), BADSTATE, "block must be opened before call to memcpy");
    panic_if((dst->zone == NULL), BADINTERNAL, "block is opened but pointer to zone is null");
    assert (page_hot_store_has(dst->manager, dst->page_id));
    assert (offset + num <= frame_store_frame_by_id(page_hot_store_fetch(dst->manager, dst->page_id), dst->frame_id)->elem_size);
    zone_memcpy(dst->zone, offset, src, num);
}

bool buffer_manager_free(
    buffer_manager_t *manager)
{
    expect_non_null(manager, false);
    expect_non_null(manager->page_hot_store, false);
    bool free_page_reg = dict_free(manager->page_hot_store);
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
    buffer_manager_t *manager)
{
    require_non_null(manager);
    manager->page_anticache.hot_store_page_ids  = vector_create_ex(sizeof(page_id_t),
                                                                   ANTICACHE_HOTSTORELIST_INITCAP,
                                                                   auto_resize,
                                                                   ANTICACHE_HOTSTORELIST_GROW_FACTOR);
    manager->page_anticache.cold_store_page_ids = vector_create_ex(sizeof(page_id_t),
                                                                   ANTICACHE_COLDSTORELIST_INITCAP,
                                                                   auto_resize,
                                                                   ANTICACHE_COLDSTORELIST_GROW_FACTOR);
    manager->page_anticache.free_page_ids_stack = vector_create_ex(sizeof(page_id_t),
                                                                   ANTICACHE_PAGEID_FREELIST_INITCAP,
                                                                   auto_resize,
                                                                   ANTICACHE_PAGEID_FREELIST_GROW_FACTOR);
    manager->page_anticache.next_page_id = 0;
    require((manager->page_anticache.hot_store_page_ids), BADHOTSTOREINIT);
    require((manager->page_anticache.cold_store_page_ids), BADCOLDSTOREINIT);
    require((manager->page_anticache.free_page_ids_stack), BADFREELISTINIT);
}

void page_anticache_page_new(
    buffer_manager_t *manager,
    size_t size)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_page_new));   // TODO: Implement
}

page_t *page_anticache_get_page_by_id(buffer_manager_t *manager, page_id_t page_id)
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
    buffer_manager_t *manager,
    page_id_t page_id)
{
    page_anticache_get_page_by_id(manager, page_id);
}

void page_anticache_page_delete(
    buffer_manager_t *manager,
    page_t *page)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_page_delete));   // TODO: Implement
}

void page_anticache_free_list_push(
    buffer_manager_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_free_list_push));   // TODO: Implement
}

bool page_anticache_free_list_is_empty(
    buffer_manager_t *manager)
{
    assert (manager);
    return (manager->page_anticache.free_page_ids_stack->num_elements == 0);
}

page_t *page_anticache_create_page(
    buffer_manager_t *manager)
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
    buffer_manager_t *manager)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_free_list_pop));   // TODO: Implement
}

size_t page_anticache_new_page_id(buffer_manager_t *manager)
{
    require_non_null(manager);
    return manager->page_anticache.next_page_id++;
}

void page_anticache_hot_store_add(
    buffer_manager_t *manager,
    page_t *page)
{
    warn(NOTIMPLEMENTED, to_string(page_anticache_hot_store_add));   // TODO: Implement

    // and register
    page_hot_store_set(manager, page->page_header.page_id, page);
}

void page_anticache_hot_store_remove(
    buffer_manager_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_hot_store_remove));   // TODO: Implement
}

page_id_t page_anticache_hot_store_iterate(
    buffer_manager_t *manager,
    page_id_t last)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_hot_store_iterate));   // TODO: Implement
}

bool page_anticache_hot_store_is_empty(
    buffer_manager_t *manager)
{
    assert (manager);
    return (manager->page_anticache.hot_store_page_ids->num_elements == 0);
}

void page_anticache_cold_store_add(
    buffer_manager_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_cold_store_add));   // TODO: Implement
}

void page_anticache_cold_store_remove(
    buffer_manager_t *manager,
    page_id_t page_id)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_cold_store_remove));   // TODO: Implement
}

page_id_t page_anticache_cold_store_iterate(
    buffer_manager_t *manager,
    page_id_t last)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_cold_store_iterate));   // TODO: Implement
}

bool page_anticache_cold_store_is_empty(
    buffer_manager_t *manager)
{
    assert (manager);
    return (manager->page_anticache.cold_store_page_ids->num_elements == 0);
}

void page_anticache_free(
    buffer_manager_t *manager)
{
    panic(NOTIMPLEMENTED, to_string(page_anticache_free));   // TODO: Implement
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

bool page_hot_store_init(
    buffer_manager_t *manager)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager == NULL), BADARGZERO, to_string(num_pages));
    panic_if(!dict_empty(manager->page_hot_store), BADPOOLINIT, manager);
    return true;
}

void page_hot_store_set(
    buffer_manager_t *manager,
    page_id_t id,
    void *ptr)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager->page_hot_store == NULL), UNEXPECTED, "page register in manager");
    warn_if((page_hot_store_has(manager, id)), UNEXPECTED, "page is already registered in hot store");
    dict_put(manager->page_hot_store, &id, &ptr);
}

bool page_hot_store_has(
    buffer_manager_t *manager,
    page_id_t id)
{
    return (page_hot_store_fetch(manager, id) != NULL);
}

bool page_hot_store_remove(
    buffer_manager_t *manager,
    page_id_t id)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager->page_hot_store == NULL), UNEXPECTED, "page register in manager");
    panic_if (!(dict_contains_key(manager->page_hot_store, &id)), BADPAGEID, id);
    return dict_remove(manager->page_hot_store, 1, &id);
}

page_t *page_hot_store_fetch(
        buffer_manager_t *manager,
        page_id_t id)
{
    panic_if((manager == NULL), BADARGNULL, to_string(manager));
    panic_if((manager->page_hot_store == NULL), UNEXPECTED, "page register in manager");

    const void *page_ptr = dict_get(manager->page_hot_store, &id);
    void **element = (void **)page_ptr;
    return (element != NULL ? *element : NULL);
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

void page_cold_store_init(
    buffer_manager_t *manager)
{

}

void page_cold_store_clear(
    buffer_manager_t *manager)
{
    panic(NOTIMPLEMENTED, to_string(page_cold_store_clear));   // TODO: Implement
}

page_t *page_cold_store_fetch(
    buffer_manager_t *manager,
    page_id_t id)
{
    assert (manager);
    page_t *loaded_page = manager->page_cold_store.fetch(id);
    page_anticache_hot_store_add(manager, loaded_page);
    return loaded_page;
}

void page_cold_store_push(
    buffer_manager_t *manager,
    page_t *page)
{
    panic(NOTIMPLEMENTED, to_string(page_cold_store_push));   // TODO: Implement
}

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   F U N C T I O N S
// ---------------------------------------------------------------------------------------------------------------------

page_t *page_create(
    buffer_manager_t *manager,
    page_id_t id,
    size_t size,
    page_flags flags,
    size_t free_space,
    size_t frame_reg)
{
    size_t header_size = sizeof_total_hdr(free_space, frame_reg);
    size_t min_page_size = header_size + FRAME_HDR_SIZE + MIN_DATA_SIZE;

    size_t free_space_begin_offset = header_size;
    size_t free_space_size = (size - header_size);
    size_t free_space_end_offset = free_space_begin_offset + free_space_size;

    warn_if((size < MIN_DATA_SIZE), "requested page page_size %zu bytes is too small. Must be at least %zu bytes.\n",
            size, min_page_size)
    expect_greater(size, min_page_size, NULL)
    expect_greater(free_space, 0, NULL)
    expect_greater(frame_reg, 0, NULL)
    expect_greater(free_space_size, 1, NULL)
    expect_non_null(manager, NULL);

    page_t *page = malloc(size);
    expect_good_malloc(page, NULL);
    page->page_header.page_id = id;
    page->page_header.page_size = size;
    page->page_header.free_space = 0;
    page->page_header.flags.is_dirty  = has_flag(flags, page_flag_dirty);
    page->page_header.flags.is_fixed  = has_flag(flags, page_flag_fixed);
    page->page_header.flags.is_locked = has_flag(flags, page_flag_locked);
    page->frame_register.free_list_len = page->frame_register.in_use_num = page->free_space_register.list_len = 0;
    page->frame_register.max_num_frames = frame_reg;
    page->free_space_register.list_max = free_space;

    frame_store_init(page, frame_reg);
    free_store_push(page, free_space_begin_offset, free_space_end_offset);

    page_hot_store_set(manager, id, page);

    return page;
}

size_t page_get_free_space(
    page_t *page,
    free_space_get_strategy strategy)
{
    expect_non_null(page, 0);
    switch (strategy) {
        case free_space_get_quickapprox:
            return free_store_free_space_get(page);
        case free_space_get_slowexact:
            return free_store_get_max_free_range_capacity(page);
        default: {
            panic(MSG_BADBRANCH, page);
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
    block_positioning strategy,
    size_t element_size)
{
    fid_t *handle = NULL;

    expect_non_null(page, NULL);
    expect_greater(element_size, 0, NULL);

    if (!frame_store_is_full(page)) {
        handle = malloc (sizeof(frame_store_t));
        expect_good_malloc(handle, NULL);
        if ((handle->frame_id = frame_store_create(page, strategy, element_size)) == NULL_FID) {
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
    buffer_manager_t *manager,
    page_t *frame_page,
    frame_id_t frame_id,
    page_t *new_zone_page,
    block_positioning strategy)
{
    zone_t              *retval       = NULL;
    frame_t             *frame        = frame_store_frame_by_id(frame_page, frame_id);
    const size_t         block_size   = (sizeof(zone_t) + frame->elem_size);
    range_t              free_range;

    if (free_store_bind(&free_range, new_zone_page, block_size, strategy)) {
        zone_t           new_zone            = { .next = null_ptr() };
        const offset_t   new_zone_offset     = free_range.begin;
        ptr_type         dist_frame_to_new   = page_equals(frame_page, new_zone_page) ? ptr_type_near : ptr_type_far;
        bool             is_first_zone       = in_page_ptr_is_null(&frame->first);

        assert (ptr_distance(free_range.begin, free_range.end) >= block_size);

        if (is_first_zone) {
            assert (in_page_ptr_is_null(&frame->last));
            in_page_ptr_make(&new_zone.prev, frame_page, dist_frame_to_new, target_frame, ptr_distance(frame_page, frame));
            in_page_ptr_make(&frame->first, new_zone_page, dist_frame_to_new, target_zone, new_zone_offset);
        } else {
            // TODO: Implement partial update in cold store in case one of these pages are not in hot store
            // This step requires to hold max 3 pages in memory (the frame page itself, the page with the last new_zone
            // and (in worst case) an additional page for the last new_zone. To avoid loaded one page from the cold store
            // into the hot store, an alternative can be to apply "partial update" directly in the cold store to
            // update the pointers. With "partial update", no additional page must be loaded from cold store.
            assert (!in_page_ptr_is_null(&frame->last));

            zone_t       *last_zone        = ptr_cast_zone(manager, &frame->last);
            page_t       *last_zone_page   = page_anticache_get_page_by_id(manager, frame->last.page_id);
            page_anticache_pin_page(manager, last_zone_page);
            ptr_type      dist_last_to_new = page_equals(last_zone_page, new_zone_page) ? ptr_type_near : ptr_type_far;

            in_page_ptr_make(&new_zone.prev, last_zone_page, dist_last_to_new, target_zone, frame->last.offset);
            in_page_ptr_make(&last_zone->next, new_zone_page, dist_last_to_new, target_zone, new_zone_offset);
            page_anticache_unpin_page(manager, last_zone_page);
        }

        in_page_ptr_make(&frame->last, new_zone_page, dist_frame_to_new, target_zone, new_zone_offset);
        data_store_write(new_zone_page, new_zone_offset, &new_zone, sizeof(zone_t));
        //memset(data_store_at_unsafe(frame_page, new_zone_offset + sizeof(zone_t)), 'X',
        //       frame->elem_size);   // TODO: Remove this line; its just for debugging purposes
        retval = data_store_at_unsafe(new_zone_page, new_zone_offset);
    } else {
        error(err_limitreached);
    }

    return retval;
}

zone_t *buf_zone_head(
    buffer_manager_t *manager,
    page_t *page,
    frame_id_t frame_id)
{
    require_non_null(manager);
    require_non_null(page);

    frame_t *frame = frame_store_frame_by_id(page, frame_id);
    return (in_page_ptr_is_null(&frame->first) ? NULL : ptr_cast_zone(manager, &frame->first));
}

zone_t *buf_zone_next(
    buffer_manager_t *manager,
    zone_t *zone)
{
    require_non_null(manager);
    require_non_null(zone);
    if (in_page_ptr_is_null(&zone->next)) {
        return NULL;
    } else {
        page_anticache_get_page_by_id(manager, zone->next.page_id);
        return ptr_cast_zone(manager, in_page_ptr_deref(manager, &zone->next));
    }
}

bool zone_memcpy(
    zone_t *zone,
    size_t offset,
    const void *data,
    size_t size)
{
    expect_non_null(zone, false);
    expect_non_null(data, false);
    expect_greater(size, 0, false);

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
    buffer_manager_t *manager,
    page_t *page,
    const zone_t *zone)
{
    expect_non_null(page, false);
    expect_non_null(page, zone);
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

    panic_if(in_page_ptr_is_null(&zone->prev), MSG_BADBACKLINK, zone);
    panic_if((!in_page_ptr_is_null(&zone->next) && !(in_page_ptr_get_type(&zone->next) == target_zone)), MSG_BADFRWDLINK, zone);
    panic_if(!in_range(ptr_target, in_page_ptr_get_type(&zone->prev), target_frame, target_zone), MSG_BADTYPE, &zone->prev);

    bool prev_is_near  = (in_page_ptr_has_scope(&zone->prev, type_near_ptr));
    bool next_is_near  = (in_page_ptr_has_scope(&zone->next, type_near_ptr));
    bool prev_is_frame = (in_page_ptr_get_type(&zone->prev) == target_frame);
    bool prev_is_zone  = (in_page_ptr_get_type(&zone->prev) == target_zone);
    bool next_is_null  = (in_page_ptr_is_null(&zone->next));

    bool in_page_zone_is_lonely = (prev_is_frame && prev_is_near && next_is_null);
    bool in_page_zone_is_head   = (prev_is_frame && !next_is_null && prev_is_near && next_is_near);
    bool in_page_zone_is_tail   = (next_is_null && prev_is_near && prev_is_zone);
    bool in_page_zone_is_middle = (!next_is_null && prev_is_near && next_is_near && prev_is_zone);

    if (!(in_page_zone_is_lonely | in_page_zone_is_head | in_page_zone_is_tail | in_page_zone_is_middle)) {
        error(err_notincharge);
        return false;
    } else {
        frame_t *frame = find_frame(manager, zone);

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
            frame->first = null_ptr();
            frame->last = null_ptr();
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
            size_t next_zone_offset = ptr_distance(page, next_zone);

            frame->first = zone->next;
            in_page_ptr_make_near(&next_zone->prev, page, target_frame, ptr_distance(page, frame));
            if (frame->last.offset == next_zone_offset) {
                in_page_ptr_make_near(&frame->last, page, target_zone, next_zone_offset);
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
            prev_zone->next = null_ptr();
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
        } else panic(MSG_BADBRANCH, zone);

        free_store_push(page, ptr_distance(page, zone), ptr_distance(page, ((void *)zone + sizeof(zone_t) + frame->elem_size)));
        free_store_rebuild(page);
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
    buffer_manager_t *manager,
    const page_t *page)
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
        printf("# %#010lx  [free space register]\n",     ptr_distance(page, free_space_reg));
        printf("# %#010lx    capacity: %u\n",            ptr_distance(page, &free_space_reg->list_max),
                                                            free_space_reg->list_max);
        printf("# %#010lx    page_size: %u\n",                ptr_distance(page, &free_space_reg->list_len),
                                                            free_space_reg->list_len);
        printf("# %#010lx  [frame register]\n",          ptr_distance(page, frame_reg));
        printf("# %#010lx    capacity: %u\n",            ptr_distance(page, &frame_reg->max_num_frames),
                                                            frame_reg->max_num_frames);
        printf("# %#010lx    in-use page_size: %u\n",         ptr_distance(page, &frame_reg->in_use_num),
                                                            frame_reg->in_use_num);
        printf("# %#010lx    free-list page_size: %u\n",      ptr_distance(page, &frame_reg->free_list_len),
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
            printf("# %#010lx    elem_size:%zu, first:(far_ptr:%d, pid=%d, offset=%#010lx), "
                   "last:(far_ptr:%d, pid=%d, offset=%#010lx) \n",
                       frame_offset, frame->elem_size, frame->first.is_far_ptr,
                       frame->first.page_id, frame->first.offset, frame->last.is_far_ptr,
                       frame->last.page_id, frame->last.offset);
        }

        printf("# ---------- [ZONES]\n");
        for (frame_id_t frame_id = frame_store_scan(page, frame_inuse); frame_id != NULL_FID;
             frame_id = frame_store_scan(NULL, frame_inuse)) {

            frame_t *frame = frame_store_frame_by_id(page, frame_id);
            in_page_ptr ptr = frame_store_frame_by_id(page, frame_id)->first;
            while (!in_page_ptr_is_null(&ptr) && in_page_ptr_has_scope(&ptr, type_near_ptr)) {
                assert(ptr.page_id == page->page_header.page_id);

                const zone_t *zone = ptr_cast_zone(manager, &ptr);

                printf("# %#010lx    prev:(far_ptr:%d, pid=%d, offset=%#010lx), prev:(far_ptr:%d, pid=%d, offset=%#010lx)\n",
                       ptr_distance(page, zone), zone->prev.is_far_ptr, zone->prev.page_id, zone->prev.offset,
                        zone->next.is_far_ptr, zone->next.page_id, zone->next.offset);

                printf("# zone content  ");
                for (unsigned i = 0; i < 16; i++) {
                    printf("%02x ", i);
                }
                printf("\n");

                const void* data = (zone + 1);
                char *puffer = malloc(16);
                size_t block_end = 16;

                for (; block_end < frame->elem_size; block_end += 16) {
                    memset(puffer, 0, 16);
                    memcpy(puffer, data + block_end - 16, 16);
                    printf("# %#010lx    ", ptr_distance(page, data + block_end));
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
                    printf("# %#010lx    ", ptr_distance(page, data + block_end + 1));
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

static inline size_t sizeof_ext_hdr(
    size_t free_space_cap,
    size_t frame_reg_cap)
{
    return (free_space_cap * sizeof(range_t) + frame_reg_cap * sizeof(offset_t) + frame_reg_cap * sizeof(frame_id_t));
}

static inline size_t sizeof_total_hdr(
    size_t free_space_cap,
    size_t frame_reg_cap)
{
    return CORE_HDR_SIZE + sizeof_ext_hdr(free_space_cap, frame_reg_cap);
}

static inline void *seek(
    const page_t *page,
    seek_target target)
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

static inline free_store_t *free_store_in(
    const page_t *page)
{
    return (free_store_t *) seek(page, target_free_space_reg);
}

static inline void frame_store_init(
    page_t *page,
    size_t num_frames)
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

static inline frame_store_t *frame_store_in(
    const page_t *page)
{
    return (frame_store_t *) seek(page, target_frame_reg);
}

static inline range_t *free_store_at(
    const page_t *page,
    size_t pos)
{
    free_store_t *free_space_reg = free_store_in(page);
    assert (pos < free_space_reg->list_max);
    return seek(page, target_free_space_data) + sizeof(range_t) * pos;
}

static inline size_t free_store_len(
    const page_t *page)
{
    free_store_t *free_space_reg = free_store_in(page);
    return free_space_reg->list_len;
}

static inline bool free_store_pop(
    range_t *range,
    page_t *page)
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

static inline offset_t *frame_store_offset_of(
    const page_t *page,
    frame_id_t frame_id)
{
    assert (page);
    frame_store_t *frame_reg = frame_store_in(page);
    assert (frame_id < frame_reg->max_num_frames);
    return seek(page, target_frame_reg_inuse_data) + frame_id * sizeof(offset_t);
}

static inline frame_t *frame_store_frame_by_id(
    const page_t *page,
    frame_id_t frame_id)
{
    assert (page);
    offset_t offset = *frame_store_offset_of(page, frame_id);
    void *data = data_store_at_unsafe(page, offset);
    expect_non_null(data, NULL);
    return data;
}

static inline frame_id_t *frame_store_recycle(
    const page_t *page,
    size_t pos)
{
    frame_store_t *frame_reg = frame_store_in(page);
    assert (pos < frame_reg->max_num_frames);
    return seek(page, target_frame_reg_inuse_data) + frame_reg->max_num_frames * sizeof(offset_t) +
            pos * sizeof(frame_id_t);
}

static inline bool frame_store_is_full(
    const page_t *page)
{
    frame_store_t *frame_reg = frame_store_in(page);
    return (frame_reg->free_list_len == 0);
}

static inline frame_id_t frame_store_create(
    page_t *page,
    block_positioning strategy,
    size_t size)
{
    assert (!frame_store_is_full(page));

    frame_id_t frame_id = NULL_FID;
    range_t free_range;
    if ((free_store_bind(&free_range, page, FRAME_HDR_SIZE, strategy))) {
        frame_id = *frame_store_recycle(page, --(frame_store_in(page))->free_list_len);

        frame_t frame = {
            .first = null_ptr(),
            .last = null_ptr(),
            .elem_size = size
        };
        offset_t frame_offset = free_range.begin;
        assert (ptr_distance(free_range.begin, free_range.end) >= sizeof(frame_t));
        data_store_write(page, free_range.begin, &frame, sizeof(frame_t));
        frame_store_link(page, frame_id, frame_offset);
    }
    return frame_id;
}

static inline void frame_store_link(
    page_t *page,
    frame_id_t frame_id,
    offset_t frame_offset)
{
    assert(page);
    frame_store_t *frame_reg = frame_store_in(page);
    assert(frame_id < frame_reg->max_num_frames);
    *(offset_t *)(seek(page, target_frame_reg_inuse_data) + frame_id * sizeof(offset_t)) = frame_offset;
}

static inline range_t *free_store_new(
    const page_t *page)
{
    range_t *entry = NULL;
    free_store_t *free_space_stack = free_store_in(page);
    if (free_space_stack->list_len < free_space_stack->list_max) {
        size_t entry_pos = free_space_stack->list_len++;
        entry = free_store_at(page, entry_pos);
    }
    return entry;
}

static inline size_t free_store_find_range_pos_with_capacity(
    const page_t *page,
    size_t capacity)
{
    assert (page);
    assert (capacity > 0);

    size_t entry_id_cursor = free_store_len(page);

    while (entry_id_cursor--) {
        range_t *cursor = free_store_at(page, entry_id_cursor);
        if (offset_distance(cursor->begin, cursor->end) >= capacity) {
            goto return_position;
        }
    }

    entry_id_cursor = SIZE_MAX;
return_position:

    return entry_id_cursor;
}

static inline size_t free_store_get_max_free_range_capacity(const page_t *page)
{
    assert (page);

    size_t max_capacity = 0;
    size_t entry_id_cursor = free_store_len(page);

    while (entry_id_cursor--) {
        range_t *cursor = free_store_at(page, entry_id_cursor);
        max_capacity = max(max_capacity, offset_distance(cursor->begin, cursor->end));
    }

    return max_capacity;
}

static inline bool free_store_bind(
    range_t *range,
    page_t *page,
    size_t size,
    block_positioning strategy)
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
            /*while (entry_id_cursor--) {
                range_t *cursor = free_store_at(page, entry_id_cursor);
                if (offset_distance(cursor->begin, cursor->end) >= page_size) {
                    *range = free_store_splitoff(page, entry_id_cursor, page_size);
                    break;
                }
            }*/
            entry_id_cursor = free_store_find_range_pos_with_capacity(page, size);
            if (entry_id_cursor != SIZE_MAX)
                *range = free_store_splitoff(page, entry_id_cursor, size);

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

static inline range_t free_store_splitoff(
    page_t *page,
    size_t pos,
    size_t size)
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
    free_store_free_space_dec(page, size);
    return result;
}

static inline void free_store_rebuild(
    page_t *page)
{
    assert (page);
    free_store_unempty(page);
    free_store_merge(page);
}

static inline void free_store_unempty(
    page_t *page)
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

static inline int comp_range_start(
    const void *lhs,
    const void *rhs)
{
    offset_t a = ((range_t *) lhs)->begin;
    offset_t b = ((range_t *) rhs)->begin;
    return (a == b ? 0 : (a < b) ? - 1 : 1);
}

static inline void free_store_merge(
    page_t *page)
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
    vector_add(stack, 1, raw_data); // ... stack is buggy

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

static inline frame_id_t frame_store_scan(
    const page_t *page,
    frame_state state)
{
    static size_t frame_id;
    static page_t *last_page = NULL;
    if (page != NULL) {
        last_page = (page_t *) page;
        frame_id = frame_store_in(last_page)->max_num_frames;
    }

    while (frame_id--) {
        offset_t frame_offset = *frame_store_offset_of(last_page, frame_id);
        if ((state == frame_inuse && frame_offset != NULL_OFFSET) ||
            (state == frame_free  && frame_offset == NULL_OFFSET))
            return frame_id;
    }

    return NULL_FID;
}

static inline bool in_page_ptr_is_null(
    const in_page_ptr *ptr)
{
    assert(ptr);
    return (ptr->offset == NULL_OFFSET);
}

static inline bool in_page_ptr_has_scope(
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

static inline void in_page_ptr_make(
    in_page_ptr *ptr,
    page_t *page,
    ptr_type type,
    ptr_target target,
    offset_t offset)
{
    assert(ptr);
    assert(page);
    assert(offset != NULL_OFFSET);
    ptr->page_id = page->page_header.page_id;
    ptr->is_far_ptr = (type == ptr_type_far);
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

static inline void in_page_ptr_make_near(
    in_page_ptr  *ptr,
    page_t       *page,
    ptr_target   target,
    offset_t     offset)
{
    return in_page_ptr_make(ptr, page, ptr_type_near, target, offset);
}

static inline ptr_target in_page_ptr_get_type(
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

static inline void *in_page_ptr_deref(
    buffer_manager_t *buffer_manager,
    const in_page_ptr *ptr)
{
    panic_if((buffer_manager == NULL), BADARGNULL, to_string(buffer_manager));
    panic_if((ptr == NULL), BADARGNULL, to_string(ptr));
    panic_if((buffer_manager->page_hot_store == NULL), UNEXPECTED, "page register hash table is null");
    void *page_base_ptr = page_hot_store_fetch(buffer_manager, ptr->page_id);
    panic_if((page_base_ptr == NULL), UNEXPECTED, "page base pointer is not allowed to be null");
    return (page_base_ptr + ptr->offset);
}

static inline zone_t *ptr_cast_zone(
    buffer_manager_t *buffer_manager,
    const in_page_ptr *ptr)
{
    panic_if(in_page_ptr_get_type(ptr) != target_zone, MSG_BADCAST, ptr);
    void *data = in_page_ptr_deref(buffer_manager, ptr);
    return (zone_t *) data;
}

static inline frame_t *ptr_cast_frame(
    buffer_manager_t *buffer_manager,
    const in_page_ptr *ptr)
{
    panic_if(in_page_ptr_get_type(ptr) != target_frame, MSG_BADCAST, ptr);
    void *data = in_page_ptr_deref(buffer_manager, ptr);
    return (frame_t *) data;
}

static inline frame_t *find_frame(
    buffer_manager_t *manager,
    const zone_t *zone)
{
    while (!in_page_ptr_is_null(&zone->prev) && in_page_ptr_get_type(&zone->prev) != target_frame) {
        zone = ptr_cast_zone(manager, &zone->prev);
        panic_if(!in_page_ptr_has_scope(&zone->prev, type_near_ptr), BADHURRY, "Traversing over page boundaries "
                "not implemented!");
    }
    return ptr_cast_frame(manager, &zone->prev);
}

static inline void data_store_write(
    page_t *page,
    offset_t offset,
    const void *data,
    size_t size)
{
    assert (page);
    assert (offset >= ptr_distance(page, seek(page, target_free_space_begin)));
    assert (data);
    assert (size > 0);

    void *base = (void *) page;
    memcpy(base + offset, data, size);
}

static inline void *data_store_at_unsafe(
    const page_t *page,
    offset_t offset)
{
    assert (page);
    assert (offset >= ptr_distance(page, seek(page, target_free_space_begin)));
    return offset != NULL_OFFSET ? (void *) page + offset : NULL;
}

static inline bool range_do_overlap(
    range_t *lhs,
    range_t *rhs)
{
    return (lhs->end >= rhs->begin && lhs->begin <= rhs->begin) ||
           (rhs->end >= lhs->begin && rhs->begin <= lhs->begin);
}

static inline void free_store_free_space_inc(
    page_t *page,
    size_t size)
{
    assert (page);
    page->page_header.free_space += size;
}

static inline void free_store_free_space_dec(
    page_t *page,
    size_t size)
{
    assert (page);
    page->page_header.free_space -= size;
}

static inline size_t free_store_free_space_get(
    const page_t *page)
{
    return page->page_header.free_space;
}

static inline bool free_store_push(
    page_t *page,
    offset_t begin,
    offset_t end)
{
    range_t *entry = NULL;
    if ((entry = free_store_new(page))) {
        expect_non_null(entry, false);
        entry->begin = begin;
        entry->end = end;
    }
    panic_if((entry == NULL), UNEXPECTED, "Free store capacity exceeded");
    free_store_free_space_inc(page, offset_distance(begin, end));
    return (entry != NULL);
}