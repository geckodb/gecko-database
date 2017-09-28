// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
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
#include <msg.h>
#include <require.h>
#include <storage/memory.h>
#include <sys/stat.h>
#include <conf.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define NULL_LANE_ID     UINT_MAX
#define MAX_OFFSET       SIZE_MAX
#define NULL_OFFSET      0
#define CORE_HDR_SIZE    (sizeof(page_header_t) + sizeof(freespace_reg_t) + sizeof(lane_reg_t))
#define LANE_HDR_SIZE    (sizeof(lane_t))
#define MIN_DATA_SIZE    (sizeof(zone_t) + sizeof(in_page_ptr))
#define FRESH_PAGE_FLAGS 0

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
        offset_t      begin;
        offset_t      end;
} range_t;

typedef enum
{
        TARGET_LANE,
        TARGET_ZONE,
        TARGET_USERDATA,
        TARGET_CORRUPTED,
} ptr_target;

typedef enum {
        PAGE_FLAG_DIRTY     = 1 << 1,
        PAGE_FLAG_FIXED     = 1 << 2,
        PAGE_FLAG_LOCKED    = 1 << 3
} page_flags;

typedef struct __force_packing__ {
        in_page_ptr   first;
        in_page_ptr   last;
        size_t        elem_size;
} lane_t;

typedef struct {
        page_id_t     page_id;
        lane_id_t     lane_id;
} lane_handle_t;

typedef enum {
        LANE_STATE_INUSE,
        LANE_STATE_FREE
} lane_state;

typedef enum {
        FREE_SPACE_GET_APPROX,
        FREES_PACE_GET_EXACT
} free_space_get_strat;

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

typedef enum {
        MEM_SPACE_VM,
        MEM_SPACE_FILE
} mem_space;

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
        block_pos         strat
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
        size_t            lane_reg_cap
);

static inline size_t
page_total_header_sizeof(
        size_t            free_space_cap,
        size_t            lane_reg_cap
);

static inline page_t *
page_create(
        anti_buf_t *      buf,
        page_id_t         id,
        size_t            size,
        page_flags        flags,
        mem_space         mspace,
        size_t            freespace_reg_cap,
        size_t            lane_reg_cap
);

static inline size_t
page_get_free_space(
        page_t *          page,
        free_space_get_strat strat
);

static inline lane_handle_t *
lane_create(
        page_t *page,
        block_pos strat,
        size_t elem_size
);

static inline bool
page_equals(
        page_t *          lhs,
        page_t *          rhs
);

static inline void
page_dump(
        FILE *            out,
        anti_buf_t *      buf,
        const page_t *    page,
        bool              hex_view
);

// - Z O N E  ----------------------------------------------------------------------------------------------------------

static inline zone_t *
zone_create(
        anti_buf_t *      buf,
        page_t *          lane_page,
        lane_id_t         id,
        page_t *          new_zone_page,
        block_pos         strat
);

static inline zone_t *
zone_first(
        anti_buf_t *      buf,
        page_t *          page,
        lane_id_t         id
);

static inline zone_t *
zone_next(
        anti_buf_t *      buf,
        zone_t *          zone
);

static inline bool
zone_memcpy(
        zone_t *          zone,
        size_t            offset,
        const void *      data,
        size_t            size
);

static inline void *
zone_get_data(
        zone_t *          zone
);

//static inline bool
//zone_rem(
//        anti_buf_t *      buf,
//        page_t *          page,
//        const zone_t *    zone
//);

// - P A G E  P O I N T E R --------------------------------------------------------------------------------------------

static inline bool
ptr_is_null(
        const in_page_ptr *ptr
);

static inline bool
ptr_has_scope(
        const in_page_ptr *ptr,
        page_ptr_type      type
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
        anti_buf_t *       buf,
        const in_page_ptr *ptr
);

static inline zone_t *
ptr_cast_zone(
        anti_buf_t *       buf,
        const in_page_ptr *ptr
);

//static inline lane_t *
//ptr_cast_frame(
//        anti_buf_t * buffer_manager,
//        const in_page_ptr *ptr
//);

// - L A N E   R E G I S T E R  ----------------------------------------------------------------------------------------

//static inline lane_t *
//lane_by_zone(
//        anti_buf_t *manager,
//        const zone_t     *zone
//);

static inline void
lane_reg_init(
        page_t *          page,
        size_t            num_lanes
);

static inline lane_reg_t *
lane_reg(
        const page_t *    page
);

static inline offset_t *
lane_offsetof(
        const page_t *    page,
        lane_id_t         frame_id
);

static inline lane_t *
lane_by_id(
        const page_t *    page,
        lane_id_t         frame_id
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
        block_pos         strat,
        size_t            size
);

static inline void
lane_reg_link(
        page_t *          page,
        lane_id_t         frame_id,
        offset_t          lane_offset
);

static inline lane_id_t
lane_reg_scan(
        const page_t *    page,
        lane_state        state
);

static inline bool
range_do_overlap(
        range_t *         lhs,
        range_t *         rhs);

// - A N T I - B U F F E R ---------------------------------------------------------------------------------------------

static inline void
buf_init(
        anti_buf_t *      buf
);

static inline page_t *
generic_store_get(
        anti_buf_t *      buf,
        size_t            size,
        const page_id_t * (*iterate)(anti_buf_t *, const page_id_t *),
        page_t *          (*fetch)(anti_buf_t *, page_id_t),
        size_t            (*get_capacity)(page_t *, free_space_get_strat)
);

static inline void
anticache_pin(
        anti_buf_t *      buf,
        page_t *          page
);

static inline void
anticache_unpin(
        anti_buf_t *      buf,
        page_t *          page);

// - A N T I - C A C H E -----------------------------------------------------------------------------------------------

static inline void
anticache_init(
        anti_buf_t *      buf
);

static inline page_t *
anticache_page_by_id(
        anti_buf_t *      buf,
        page_id_t         id
);

static inline void
anticache_freelist_init(
        anti_buf_t *      buf
);

static inline void
anticache_guarantee_page(
        anti_buf_t *      buf,
        page_id_t         id
);

static inline bool
anticache_is_freelist_empty(
        anti_buf_t *      buf
);

static inline page_t *
anticache_create_page_safe(
        anti_buf_t *      buf,
        size_t            min_payload_size
);

static inline page_id_t
anticache_freelist_pop(
        anti_buf_t *      buf
);

static inline size_t
anticache_new_page_id(
        anti_buf_t *      buf
);

static inline void
anticache_add_page_safe(
        anti_buf_t *      buf,
        page_t *          page
);

static inline page_t *
anticache_page_by_freesize(
        anti_buf_t *      buf,
        size_t            size,
        page_t *          favored
);

static inline lane_id_t
anticache_create_lane(
        anti_buf_t *      buf,
        page_t *          page,
        block_pos         strat,
        size_t            size
);

// - H O T S T O R E ---------------------------------------------------------------------------------------------------

static inline void
hotstore_init(
        anti_buf_t *      buf
);

static inline void
hotstore_set_unsafe(
        anti_buf_t *      buf,
        page_id_t         id,
        page_t *          page
);

static inline bool
hotstore_has_unsafe(
        anti_buf_t *      buf,
        page_id_t         id
);

//static inline bool
//hotstore_rem_unsafe(
//        anti_buf_t *      buf,
//        page_id_t         tuplet_id
//);

static inline page_t *
hotstore_get_unsafe(
        anti_buf_t *      buf,
        page_id_t         id
);

static inline const page_id_t *
hotstore_iterate(
        anti_buf_t *      buf,
        const page_id_t * last
);

static inline bool
hotstore_is_empty(
        anti_buf_t *      buf
);

// - C O L D S T O R E -------------------------------------------------------------------------------------------------

static inline void
coldstore_init(
        anti_buf_t *      buf
);

//static inline void
//coldstore_clear(
//        anti_buf_t *      buf
//);

static inline page_t *
coldstore_fetch(
        anti_buf_t *      buf,
        page_id_t         id
);

//static inline void
//coldstore_push(
//        anti_buf_t *      buf,
//        page_t *          page
//);
//
//static inline void
//coldstore_add(
//        anti_buf_t *      buf,
//        page_id_t         tuplet_id
//);
//
//static inline void
//coldstore_rem(
//        anti_buf_t *      buf,
//        page_id_t         tuplet_id
//);

static inline const page_id_t *
coldstore_iterate(
        anti_buf_t *      buf,
        const page_id_t * last
);

static inline bool
coldstore_is_empty(
        anti_buf_t *      buf
);

//static inline void
//anticache_free(
//        anti_buf_t *      buf
//);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

anti_buf_t *
buf_create()
{
    anti_buf_t *result = GS_REQUIRE_MALLOC(sizeof(anti_buf_t));
    EXPECT_GOOD_MALLOC(result, NULL);

    buf_init(result);
    anticache_init(result);

    return result;
}

cursor_t *
buf_alloc(
    anti_buf_t *   buf,
    size_t         size,
    size_t         nlanes,
    block_pos      strat)
{
    EXPECT_NONNULL(buf, NULL);
    EXPECT_GREATER(size, 0, NULL);
    EXPECT_GREATER(nlanes, 0, NULL);

    cursor_t *    result    = GS_REQUIRE_MALLOC(sizeof(cursor_t));
    page_t *      page_lane = anticache_page_by_freesize(buf, LANE_HDR_SIZE, NULL);
    lane_id_t     lane      = anticache_create_lane(buf, page_lane, strat, size);

    anticache_pin(buf, page_lane);
    page_t *      last_page_written_to = page_lane;

    while (nlanes--) {
        page_t *  page_zone = anticache_page_by_freesize(buf, size, last_page_written_to);
        zone_t *  new_zone = NULL;

        last_page_written_to = page_zone;
        anticache_pin(buf, page_zone);
        nlanes += ((new_zone = zone_create(buf, page_lane, lane, page_zone, strat)) == NULL);
        anticache_unpin(buf, page_zone);
    }

    anticache_unpin(buf, page_lane);

    *result = (cursor_t) {
        .page_id   = page_lane->header.id,
        .lane_id  = lane,
        .manager   = buf,
        .zone      = NULL,
        .state     = block_state_closed,
    };

    return result;
}

void
buf_release(
    cursor_t *     cur)
{
    GS_REQUIRE_NONNULL(cur);
    assert ((cur->state != block_state_opened) || (cur->zone != NULL));
    free (cur);
}

void
buf_open(
    cursor_t *     cur)
{
    GS_REQUIRE_NONNULL(cur);
    GS_REQUIRE_NONNULL(cur->manager);
    panic_if((cur->state == block_state_opened), BADSTATE, "block was already opened.")
    cur->state = block_state_opened;
}

bool buf_next(
    cursor_t *     cur)
{
    GS_REQUIRE_NONNULL(cur);
    GS_REQUIRE_NONNULL(cur->manager);

    if (cur->state == block_state_opened) {
        page_t *page   = anticache_page_by_id(cur->manager, cur->page_id);
        if (cur->zone == NULL) {
            cur->zone  = zone_first(cur->manager, page, cur->lane_id);
        } else {
            cur->zone  = zone_next(cur->manager, cur->zone);
            cur->state = (cur->zone == NULL ? block_state_closed : block_state_opened);
        }
        return (cur->zone != NULL ? true : false);
    } else return false;
}

void
buf_close(
    cursor_t *     cur)
{
    GS_REQUIRE_NONNULL(cur);
    cur->state = block_state_closed;
}

void
buf_read(
    cursor_t *     cur,
    void *         capture,
    void           (*consumer)(void *capture, const void *data))
{
    GS_REQUIRE_NONNULL(cur);
    GS_REQUIRE_NONNULL(consumer);
    panic_if((cur->zone == NULL), BADSTATE, "read operation on illegal zone");
    consumer (capture, zone_get_data(cur->zone));
}

void
buf_memcpy(
    cursor_t *    dst,
    size_t        offset,
    const void *  src,
    size_t size)
{
    GS_REQUIRE_NONNULL(dst);
    GS_REQUIRE_NONNULL(src);
    panic_if((dst->state != block_state_opened), BADSTATE, "block must be opened before call to memcpy");
    panic_if((dst->zone == NULL), BADINTERNAL, "block is opened but pointer to zone is null");
    assert (hotstore_has_unsafe(dst->manager, dst->page_id));
    assert (offset + size <= lane_by_id(hotstore_get_unsafe(dst->manager, dst->page_id), dst->lane_id)->elem_size);
    zone_memcpy(dst->zone, offset, src, size);
}

void
buf_dump(
    FILE *         out,
    anti_buf_t *   buf,
    bool           hex_view)
{
    for(const void *it = list_begin(buf->page_anticache.hot_store_page_ids); it != NULL; it = list_next(it)) {
        const page_id_t page_id = *(const page_id_t *) it;
        const page_t *page = *(const page_t **) dict_get(buf->page_anticache.hot_store, &page_id);

        page_dump(out, buf, page, hex_view);
    }
}

bool buf_free(
    anti_buf_t *   buf)
{
    EXPECT_NONNULL(buf, false);
    EXPECT_NONNULL(buf->page_anticache.hot_store, false);
    bool free_page_reg = dict_delete(buf->page_anticache.hot_store);
    if (free_page_reg) {
        free (buf);
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   F U N C T I O N   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

static inline void
anticache_init(
    anti_buf_t *   buf)
{
    GS_REQUIRE_NONNULL(buf);
    hotstore_init(buf);
    anticache_freelist_init(buf);
    coldstore_init(buf);
    buf->page_anticache.next_page_id = 0;
}

static inline page_t *
anticache_page_by_id(
    anti_buf_t *      buf,
    page_id_t         id)
{
    GS_REQUIRE_NONNULL(buf);
    WARN_IF((id >= buf->page_anticache.next_page_id), "Requested page tuplet_id '%d' might not from this pool", id);

    page_t *result = NULL;

    if (((result = hotstore_get_unsafe(buf, id)) == NULL) &&
        ((result = coldstore_fetch(buf, id)) == NULL)) {
        panic(BADPAGEID, id);
    }

    return result;
}

static inline void
anticache_freelist_init(
    anti_buf_t *   buf)
{
    buf->page_anticache.free_page_ids_stack = vec_new_ex(sizeof(page_id_t),
                                                         ANTICACHE_PAGEID_FREELIST_INITCAP, auto_resize,
                                                         ANTICACHE_PAGEID_FREELIST_GROW_FACTOR);
    REQUIRE((buf->page_anticache.free_page_ids_stack), BADFREELISTINIT);
}

static inline void
anticache_guarantee_page(
        anti_buf_t *buf,
        page_id_t id)
{
    anticache_page_by_id(buf, id);
}

static inline bool
anticache_is_freelist_empty(
    anti_buf_t *    buf)
{
    assert (buf);
    return (buf->page_anticache.free_page_ids_stack->num_elements == 0);
}

static inline page_t *
anticache_create_page_safe(
    anti_buf_t *    buf,
    size_t          min_payload_size)
{
    GS_REQUIRE_NONNULL(buf);
    page_t *        result             = NULL;
    size_t          matching_page_size = buf->config.ram_page_size_default;
    mem_space       mspace             = MEM_SPACE_VM;

    size_t id = !anticache_is_freelist_empty(buf) ? anticache_freelist_pop(buf) :
                                                    anticache_new_page_id(buf);

    if (min_payload_size > matching_page_size) {
        if (min_payload_size < buf->config.ram_page_size_max) {
            matching_page_size = buf->config.ram_page_size_max;
        } else {
            mspace             = MEM_SPACE_FILE;
            matching_page_size = min_payload_size;
        }
    }

    result = page_create(buf, id, matching_page_size, FRESH_PAGE_FLAGS, mspace,
                         buf->config.free_space_reg_capacity,
                         buf->config.lane_reg_capacity);

    anticache_add_page_safe(buf, result);
    return result;
}

static inline page_id_t
anticache_freelist_pop(
    anti_buf_t *buf)
{
    panic(NOTIMPLEMENTED, to_string(anticache_freelist_pop));   // TODO: Implement
}

static inline size_t
anticache_new_page_id(
    anti_buf_t *buf)
{
    GS_REQUIRE_NONNULL(buf);
    return buf->page_anticache.next_page_id++;
}

static inline void
anticache_add_page_safe(
    anti_buf_t *buf,
    page_t *page)
{
    warn(NOTIMPLEMENTED, to_string(anticache_add_page_safe));   // TODO: Implement
    // and register
    hotstore_set_unsafe(buf, page->header.id, page);
}

//static inline
//void coldstore_add(
//    anti_buf_t *buf,
//    page_id_t tuplet_id)
//{
//    panic(NOTIMPLEMENTED, to_string(coldstore_add));   // TODO: Implement
//}
//
//static inline void
//coldstore_rem(
//    anti_buf_t *buf,
//    page_id_t tuplet_id)
//{
//    panic(NOTIMPLEMENTED, to_string(coldstore_rem));   // TODO: Implement
//}

static inline const page_id_t *
coldstore_iterate(
    anti_buf_t *     buf,
    const page_id_t *last)
{
    panic(NOTIMPLEMENTED, to_string(coldstore_iterate));   // TODO: Implement
}

static inline bool
coldstore_is_empty(
    anti_buf_t *     buf)
{
    assert (buf);
    return (buf->page_anticache.cold_store_page_ids->num_elements == 0);
}

//static inline void
//anticache_free(
//        anti_buf_t *buf)
//{
//    panic(NOTIMPLEMENTED, to_string(anticache_free));   // TODO: Implement
//}

static inline void
hotstore_init(
    anti_buf_t *   buf)
{
    panic_if((buf == NULL), BADARGNULL, to_string(buf));
    panic_if((buf == NULL), BADARGZERO, to_string(num_pages));
    panic_if(!dict_empty(buf->page_anticache.hot_store), BADPOOLINIT, buf);
    buf->page_anticache.hot_store_page_ids = list_new(sizeof(page_id_t));
    REQUIRE((buf->page_anticache.hot_store_page_ids), BADCOLDSTOREINIT);
}

static inline void
hotstore_set_unsafe(
    anti_buf_t *   buf,
    page_id_t      id,
    page_t *       page)
{
    panic_if((buf == NULL), BADARGNULL, to_string(buf));
    panic_if((buf->page_anticache.hot_store == NULL), UNEXPECTED, "page register in buf");
    dict_put(buf->page_anticache.hot_store, &id, &page);
    list_push(buf->page_anticache.hot_store_page_ids, &id);
}

static inline bool
hotstore_has_unsafe(
    anti_buf_t *   buf,
    page_id_t      id)
{
    return (hotstore_get_unsafe(buf, id) != NULL);
}

//static inline bool
//hotstore_rem_unsafe(
//        anti_buf_t *buf,
//        page_id_t tuplet_id)
//{
//    panic_if((buf == NULL), BADARGNULL, to_string(buf));
//    panic_if((buf->page_anticache.hot_store == NULL), UNEXPECTED, "page register in buf");
//    panic_if (!(dict_contains_key(buf->page_anticache.hot_store, &tuplet_id)), BADPAGEID, tuplet_id);
//
//    const void *it = list_begin(buf->page_anticache.hot_store_page_ids);
//    do {
//        if (* (page_id_t *)it == tuplet_id) {
//            list_remove(it);
//            break;
//        }
//    } while ((it = list_next(it)));
//
//    return dict_remove(buf->page_anticache.hot_store, 1, &tuplet_id);
//}

static inline page_t *
hotstore_get_unsafe(
    anti_buf_t *   buf,
    page_id_t      id)
{
    panic_if((buf == NULL), BADARGNULL, to_string(buf));
    panic_if((buf->page_anticache.hot_store == NULL), UNEXPECTED, "page register in buf");

    const void *page_ptr = dict_get(buf->page_anticache.hot_store, &id);
    void **element = (void **)page_ptr;
    assert (element != NULL); // TODO: Remove if cold store works
    return (element != NULL ? *element : NULL);
}

static inline const page_id_t*
hotstore_iterate(
    anti_buf_t *     buf,
    const page_id_t *last)
{
    return (last == NULL? list_begin(buf->page_anticache.hot_store_page_ids) :
            list_next(last));
}

static inline bool
hotstore_is_empty(
    anti_buf_t *   buf)
{
    assert (buf);
    return (dict_empty(buf->page_anticache.hot_store));
}

static inline void
coldstore_init(
    anti_buf_t *   buf)
{
    buf->page_anticache.cold_store_page_ids = vec_new_ex(sizeof(page_id_t),
                                                         ANTICACHE_COLDSTORELIST_INITCAP, auto_resize,
                                                         ANTICACHE_COLDSTORELIST_GROW_FACTOR);
    REQUIRE((buf->page_anticache.cold_store_page_ids), BADCOLDSTOREINIT);

}

//static inline void
//coldstore_clear(
//        anti_buf_t *buf)
//{
//    panic(NOTIMPLEMENTED, to_string(coldstore_clear));   // TODO: Implement
//}

static inline page_t *
coldstore_fetch(
    anti_buf_t *   buf,
    page_id_t      id)
{
    assert (buf);
    page_t *loaded_page = buf->page_anticache.cold_store.fetch(id);
    anticache_add_page_safe(buf, loaded_page);
    return loaded_page;
}

//static inline void
//coldstore_push(
//        anti_buf_t *buf,
//        page_t *page)
//{
//    panic(NOTIMPLEMENTED, to_string(coldstore_push));   // TODO: Implement
//}

static inline page_t *
page_create(
    anti_buf_t *   buf,
    page_id_t      id,
    size_t         size,
    page_flags     flags,
    mem_space      mspace,
    size_t         freespace_reg_cap,
    size_t         lane_reg_cap)
{
    size_t         header_size              = page_total_header_sizeof(freespace_reg_cap, lane_reg_cap);
    size_t         min_page_size            = header_size + LANE_HDR_SIZE + MIN_DATA_SIZE;
    size_t         free_space_begin_offset  = header_size;
    size_t         free_space_size          = (size - header_size);
    size_t         free_space_end_offset    = free_space_begin_offset + free_space_size;

    WARN_IF((size < MIN_DATA_SIZE),
            "requested page size %zu bytes is too small. Must be at least %zu bytes.\n",
            size, min_page_size)
    EXPECT_GREATER(size, min_page_size, NULL)
    EXPECT_GREATER(freespace_reg_cap, 0, NULL)
    EXPECT_GREATER(lane_reg_cap, 0, NULL)
    EXPECT_GREATER(free_space_size, 1, NULL)
    EXPECT_NONNULL(buf, NULL);

    page_t * page = NULL;
    struct stat st = {0};

    switch (mspace) {
        case MEM_SPACE_VM:
            page = malloc(size);
            EXPECT_GOOD_MALLOC(page, NULL);
            break;
        case MEM_SPACE_FILE:
            if (stat(buf->config.swap_dir, &st) == -1) {
                mkdir(buf->config.swap_dir, 0700);
            }

         //   int file_desc = fopen()
            break;
        default:
        panic(BADBRANCH, buf);
    }

    page->header.id = id;
    page->header.size = size;
    page->header.freespace = 0;
    page->header.flags.is_dirty  = HAS_FLAG(flags, PAGE_FLAG_DIRTY);
    page->header.flags.is_fixed  = HAS_FLAG(flags, PAGE_FLAG_FIXED);
    page->header.flags.is_locked = HAS_FLAG(flags, PAGE_FLAG_LOCKED);
    page->lane_reg.free_list_len = page->lane_reg.in_use_num = page->freespace_reg.list_len = 0;
    page->lane_reg.lane_num_limit = lane_reg_cap;
    page->freespace_reg.list_max = freespace_reg_cap;

    lane_reg_init(page, lane_reg_cap);
    freespace_push(page, free_space_begin_offset, free_space_end_offset);

    return page;
}

static inline size_t
page_get_free_space(
    page_t *             page,
    free_space_get_strat strat)
{
    EXPECT_NONNULL(page, 0);
    switch (strat) {
        case FREE_SPACE_GET_APPROX:
            return page_approx_freespace(page);
        case FREES_PACE_GET_EXACT:
            return freespace_largest(page);
        default: {
            panic(BADBRANCH, page);
            return 0;
        }
    }
}

static inline lane_handle_t *
lane_create(
    page_t *       page,
    block_pos      strat,
    size_t         elem_size)
{
    lane_handle_t *handle = NULL;

    EXPECT_NONNULL(page, NULL);
    EXPECT_GREATER(elem_size, 0, NULL);

    if (!lane_reg_is_full(page)) {
        handle = malloc (sizeof(lane_reg_t));
        EXPECT_GOOD_MALLOC(handle, NULL);
        if ((handle->lane_id = lane_new(page, strat, elem_size)) == NULL_LANE_ID) {
            return NULL;
        }
        handle->page_id = page->header.id;
    } else error(err_limitreached);

    return handle;
}

static inline bool
page_equals(
    page_t  *      lhs,
    page_t  *      rhs)
{
    return ((lhs != NULL && rhs != NULL) && (lhs->header.id == rhs->header.id));
}

static inline void
page_dump(
    FILE *         out,
    anti_buf_t *   buf,
    const page_t * page,
    bool           hex_view)
{
    assert (out);
    printf("\n#\n");
    printf("# A page dump was requested by the system.\n");
    printf("# \n");
    if (page) {
        size_t free_space_reg_len = freespace(page)->list_max;
        size_t lane_reg_len = lane_reg(page)->lane_num_limit;
        size_t total_header_size = page_total_header_sizeof(free_space_reg_len, lane_reg_len);

        freespace_reg_t *free_space_reg = freespace(page);
        lane_reg_t *lanes = lane_reg(page);

        size_t page_capacity = (page->header.size - total_header_size);
        printf("# Page (%p), pid=%d, size/capacity/free=%zu/%zu/%zu byte (footprint=%.4f%%, filled=%.4f%%)\n",
               page, page->header.id,
               page->header.size,
               page_capacity,
               page->header.freespace,
               (page->header.size - page_capacity)/(float)(page->header.size) * 100,
               (page_capacity - page->header.freespace)/(float)(page_capacity) * 100);
        printf("#\n");
        printf("# Segments:\n");
        printf("# 0x%08x [HEADER]\n", 0);
        printf("# %#010lx  [free space register]\n",     PTR_DISTANCE(page, free_space_reg));
        printf("# %#010lx    capacity: %u\n",            PTR_DISTANCE(page, &free_space_reg->list_max),
               free_space_reg->list_max);
        printf("# %#010lx    size: %u\n",                PTR_DISTANCE(page, &free_space_reg->list_len),
               free_space_reg->list_len);
        printf("# %#010lx  [lane register]\n",          PTR_DISTANCE(page, lanes));
        printf("# %#010lx    capacity: %u\n",            PTR_DISTANCE(page, &lanes->lane_num_limit),
               lanes->lane_num_limit);
        printf("# %#010lx    in-use size: %u\n",         PTR_DISTANCE(page, &lanes->in_use_num),
               lanes->in_use_num);
        printf("# %#010lx    free-list size: %u\n",      PTR_DISTANCE(page, &lanes->free_list_len),
               lanes->free_list_len);
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

        printf("# %#010lx  [lane register in-use list]\n",       PTR_DISTANCE(page, lane_offsetof(page, 0)));
        for (lane_id_t lane_id = 0; lane_id < lane_reg_len; lane_id++) {
            offset_t *offset = lane_offsetof(page, lane_id);
            printf("# %#010lx    lane_id=%05u: offset=", PTR_DISTANCE(page, offset), lane_id);
            if (*offset != NULL_OFFSET) {
                printf("%#010lx\n", *offset);
            } else
                printf("(unset)\n");
        }

        printf("# %#010lx  [lane register free-list stack]\n",       PTR_DISTANCE(page, lane_reg_entry_by_pos(page, 0)));
        for (size_t idx = 0; idx < lanes->free_list_len; idx++) {
            lane_id_t *lane = lane_reg_entry_by_pos(page, idx);
            printf("# %#010lx    pos=%05zu: lane_id=%u\n", PTR_DISTANCE(page, lane), idx, *lane);
        }

        for (size_t idx = lanes->free_list_len; idx < lanes->lane_num_limit; idx++) {
            lane_id_t *lane = lane_reg_entry_by_pos(page, idx);
            printf("# %#010lx    pos=%05zu: (unset)\n", PTR_DISTANCE(page, lane), idx);
        }

        printf("# %#010lx [PAYLOAD]\n",                   PTR_DISTANCE(page, seek(page, SEEK_PAYLOAD)));

        printf("# ---------- [LANES IN USE]\n");
        for (lane_id_t lane_id2 = lane_reg_scan(page, LANE_STATE_INUSE); lane_id2 != NULL_LANE_ID;
             lane_id2 = lane_reg_scan(NULL, LANE_STATE_INUSE))
        {
            lane_t *lane = lane_by_id(page, lane_id2);
            assert (lane);

            offset_t lane_offset = *lane_offsetof(page, lane_id2);
            printf("# %#010lx    elem_size:%zu, first:(far_ptr:%d, pid=%d, offset=%#010lx), "
                           "last:(far_ptr:%d, pid=%d, offset=%#010lx) \n",
                   lane_offset, lane->elem_size, lane->first.is_far_ptr,
                   lane->first.page_id, lane->first.offset, lane->last.is_far_ptr,
                   lane->last.page_id, lane->last.offset);
        }

        printf("# ---------- [ZONES]\n");
        for (lane_id_t lane_id3 = lane_reg_scan(page, LANE_STATE_INUSE); lane_id3 != NULL_LANE_ID;
             lane_id3 = lane_reg_scan(NULL, LANE_STATE_INUSE)) {

            lane_t *lane = lane_by_id(page, lane_id3);
            in_page_ptr ptr = lane_by_id(page, lane_id3)->first;
            while (!ptr_is_null(&ptr) && ptr_has_scope(&ptr, PAGE_PTR_NEAR)) {
                assert(ptr.page_id == page->header.id);

                const zone_t *zone = ptr_cast_zone(buf, &ptr);

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

                    for (; block_end < lane->elem_size; block_end += 16) {
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

                    unsigned bytes_remain = 16 - (block_end - lane->elem_size);
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
        printf("# %#010lx end\n", page->header.size);

    } else {
        printf("# ERR_NULL_POINTER: The system was unable to fetch details: null pointer to page.\n");
    }
    printf("#\n");
}

static inline zone_t *
zone_create(
    anti_buf_t *   buf,
    page_t *       lane_page,
    lane_id_t      id,
    page_t *       new_zone_page,
    block_pos      strat)
{
    zone_t *       retval       = NULL;
    lane_t *       lane         = lane_by_id(lane_page, id);
    const size_t   block_size   = (sizeof(zone_t) + lane->elem_size);
    range_t        free_range;

    if (freespace_bind(&free_range, new_zone_page, block_size, strat)) {
        zone_t           new_zone            = { .next = PAGE_NULL_PTR() };
        const offset_t   new_zone_offset     = free_range.begin;
        page_ptr_type    dist_lane_to_new    = page_equals(lane_page, new_zone_page) ? PAGE_PTR_NEAR : PAGE_PTR_FAR;
        bool             is_first_zone       = ptr_is_null(&lane->first);

        assert (PTR_DISTANCE(free_range.begin, free_range.end) >= block_size);

        if (is_first_zone) {
            assert (ptr_is_null(&lane->last));
            ptr_make(&new_zone.prev, lane_page, dist_lane_to_new, TARGET_LANE, PTR_DISTANCE(lane_page, lane));
            ptr_make(&lane->first, new_zone_page, dist_lane_to_new, TARGET_ZONE, new_zone_offset);
        } else {
            // TODO: Implement partial update in cold store in case one of these pages are not in hot store
            // This step requires to hold max 3 pages in memory (the lane page itself, the page with the last new_zone
            // and (in worst case) an additional page for the last new_zone. To avoid loaded one page from the cold store
            // into the hot store, an alternative can be to apply "partial update" directly in the cold store to
            // update the pointers. With "partial update", no additional page must be loaded from cold store.
            assert (!ptr_is_null(&lane->last));
            zone_t *       last_zone        = ptr_cast_zone(buf, &lane->last);
            page_t *       last_zone_page   = anticache_page_by_id(buf, lane->last.page_id);
            anticache_pin(buf, last_zone_page);
            page_ptr_type  dist_last_to_new = page_equals(last_zone_page, new_zone_page) ? PAGE_PTR_NEAR : PAGE_PTR_FAR;

            ptr_make(&new_zone.prev, last_zone_page, dist_last_to_new, TARGET_ZONE, lane->last.offset);
            ptr_make(&last_zone->next, new_zone_page, dist_last_to_new, TARGET_ZONE, new_zone_offset);
            assert (ptr_cast_zone(buf, &last_zone->next) != last_zone);
            anticache_unpin(buf, last_zone_page);
        }
        ptr_make(&lane->last, new_zone_page, dist_lane_to_new, TARGET_ZONE, new_zone_offset);
        write_unsafe(new_zone_page, new_zone_offset, &new_zone, sizeof(zone_t));
        retval = read_unsafe(new_zone_page, new_zone_offset);
    } else {
        error(err_limitreached);
    }

    return retval;
}

static inline zone_t *
zone_first(
    anti_buf_t *   buf,
    page_t *       page,
    lane_id_t      id)
{
    GS_REQUIRE_NONNULL(buf);
    GS_REQUIRE_NONNULL(page);

    lane_t *      lane = lane_by_id(page, id);
    return (ptr_is_null(&lane->first) ? NULL : ptr_cast_zone(buf, &lane->first));
}

static inline zone_t *
zone_next(
    anti_buf_t *   buf,
    zone_t *       zone)
{
    GS_REQUIRE_NONNULL(buf);
    GS_REQUIRE_NONNULL(zone);
    if (ptr_is_null(&zone->next)) {
        return NULL;
    } else {
        zone_t *  nextzone = ptr_cast_zone(buf, &zone->next);
        assert (nextzone != zone);
        return nextzone;
    }
}

static inline bool
zone_memcpy(
    zone_t *       zone,
    size_t         offset,
    const void *   data,
    size_t         size)
{
    EXPECT_NONNULL(zone, false);
    EXPECT_NONNULL(data, false);
    EXPECT_GREATER(size, 0, false);

    memcpy(zone_get_data(zone) + offset, data, size);
    return true;
}

static inline void *
zone_get_data(
    zone_t *       zone)
{
    GS_REQUIRE_NONNULL(zone);
    return (zone + 1);
}

//static inline bool
//zone_rem(
//        anti_buf_t *buf,
//        page_t *page,
//        const zone_t *zone)
//{
//    EXPECT_NONNULL(page, false);
//    EXPECT_NONNULL(page, zone);
//    /*
//     *  Case | IN-NULL | OUT-NULL | IN	 | OUT  | Scope	  | Note
//     *  -----+---------+----------+------+------+---------+-------------------------
//     *  0    | LANE   | YES	  | NEAR | -    | 1 Page  |
//     *  1    | LANE   | YES	  | FAR	 | -	| 2 Pages | Handled outside this page
//     *  2    | LANE   | NO		  | NEAR | NEAR | 1 Page  |
//     *  3    | LANE   | NO		  | FAR	 | NEAR | 2 Pages | Handled outside this page
//     *  4    | LANE   | NO		  | NEAR | FAR  | 2 Pages | Handled outside this page
//     *  5    | LANE   | NO		  | FAR	 | FAR  | 3 Pages | Handled outside this page
//     *  6    | NO	   | YES	  | NEAR | -    | 1 Page  |
//     *  7    | NO	   | YES	  | FAR	 | -    | 2 Pages | Handled outside this page
//     *  8    | NO	   | NO		  | NEAR | NEAR | 1 Page  |
//     *  9    | NO	   | NO		  | FAR	 | NEAR | 2 Pages | Handled outside this page
//     *  10   | NO	   | NO		  | NEAR | FAR  | 2 Pages | Handled outside this page
//     *  11   | NO	   | NO		  | FAR	 | FAR  | 3 Pages | Handled outside this page
//     */
//
//    panic_if(ptr_is_null(&zone->prev), BADBACKLINK, zone);
//    panic_if((!ptr_is_null(&zone->next) && !(ptr_typeof(&zone->next) == TARGET_ZONE)), BADFRWDLINK, zone);
//    panic_if(!IN_RANGE(ptr_target, ptr_typeof(&zone->prev), TARGET_LANE, TARGET_ZONE), BADTYPE, &zone->prev);
//
//    bool prev_is_near  = (ptr_has_scope(&zone->prev, type_near_ptr));
//    bool next_is_near  = (ptr_has_scope(&zone->next, type_near_ptr));
//    bool prev_is_frame = (ptr_typeof(&zone->prev) == TARGET_LANE);
//    bool prev_is_zone  = (ptr_typeof(&zone->prev) == TARGET_ZONE);
//    bool next_is_null  = (ptr_is_null(&zone->next));
//
//    bool in_page_zone_is_lonely = (prev_is_frame && prev_is_near && next_is_null);
//    bool in_page_zone_is_head   = (prev_is_frame && !next_is_null && prev_is_near && next_is_near);
//    bool in_page_zone_is_tail   = (next_is_null && prev_is_near && prev_is_zone);
//    bool in_page_zone_is_middle = (!next_is_null && prev_is_near && next_is_near && prev_is_zone);
//
//    if (!(in_page_zone_is_lonely | in_page_zone_is_head | in_page_zone_is_tail | in_page_zone_is_middle)) {
//        error(err_notincharge);
//        return false;
//    } else {
//        lane_t *frame = lane_by_zone(buf, zone);
//
//        if (in_page_zone_is_lonely) {
//        /*
//         *  +---------------- THIS PAGE ----------------+       +---------------- THIS PAGE ----------------+
//         *  |     v--------- frame -----|               |       |                                           |
//         *  |  +-------+            +--------+          |       |  +-------+                                |
//         *  |  | FRAME | - first -> | ZONE X | -> NULL  |   =>  |  | FRAME | - first ->   NULL              |
//         *  |  +-------+            +--------+          |       |  +-------+                                |
//         *  |     |-------- last -------^               |       |     |------- last -------^                |
//         *  +-------------------------------------------+       +-------------------------------------------+
//         */
//            frame->first = PAGE_NULL_PTR();
//            frame->last = PAGE_NULL_PTR();
//        } else if (in_page_zone_is_head) {
//        /*
//         *  +---------------- THIS PAGE ----------------------------+       +---------------- THIS PAGE -------------+
//         *  |      v--------- frame ----|                           |       |      v--------- frame ----|            |
//         *  |  +-------+            +--------+     +------+         |       |  +-------+            +------+         |
//         *  |  | FRAME | - first -> | ZONE X | <-> | ZONE | -> ...  |   =>  |  | FRAME | - first -> | ZONE | -> ...  |
//         *  |  +-------+            +--------+     +------+         |       |  +-------+            +------+         |
//         *  |     |------- last -----------------------^?-------^   |       |     |------- last --------^?-------^   |
//         *  +-------------------------------------------------------+       +----------------------------------------+
//         */
//            zone_t *next_zone = ptr_cast_zone(buf, &zone->next);
//            size_t next_zone_offset = PTR_DISTANCE(page, next_zone);
//
//            frame->first = zone->next;
//            ptr_make(&next_zone->prev, page, PAGE_PTR_NEAR, TARGET_LANE, PTR_DISTANCE(page, frame));
//            if (frame->last.offset == next_zone_offset) {
//                ptr_make(&frame->last, page, PAGE_PTR_NEAR, TARGET_ZONE, next_zone_offset);
//            }
//        } else if (in_page_zone_is_tail) {
//        /*
//        *  +---------------- THIS PAGE -----------------------------+       +---------------- THIS PAGE -------------+
//        *  |      v- frame ----|                                    |       |      v----frame --|                    |
//        *  |  +-------+             +------+     +--------+         |       |  +-------+            +------+         |
//        *  |  | FRAME | -1st->...-> | ZONE | <-> | ZONE X | -> NULL |   =>  |  | FRAME | -1st->...->| ZONE | -> NULL |
//        *  |  +-------+             +------+     +--------+         |       |  +-------+            +------+         |
//        *  |     |------- last --------------------^                |       |     |------- last -------^             |
//        *  +--------------------------------------------------------+       +----------------------------------------+
//        */
//            zone_t *prev_zone = ptr_cast_zone(buf, &zone->prev);
//            frame->last = zone->prev;
//            prev_zone->next = PAGE_NULL_PTR();
//        } else if (in_page_zone_is_middle) {
//         /*
//         *  +---------------- THIS PAGE ----------------------------------------+       +---------------- THIS PAGE --------------------------+
//         *  |      v-frame --|                                                  |       |      v-frame --|                                    |
//         *  |  +-------+             +------+    +--------+    +------+         |       |  +-------+             +------+    +------+         |
//         *  |  | FRAME | -1st->...-> | ZONE | <> | ZONE X | <> | ZONE | -> ...  |   =>  |  | FRAME | -1st->...-> | ZONE | <> | ZONE | -> ...  |
//         *  |  +-------+             +------+    +--------+    +------+         |       |  +-------+             +------+    +------+         |
//         *  |     |------- last --------------------------------------------^   |       |     |------- last ------------------------------^   |
//         *  +-------------------------------------------------------------------+       +-----------------------------------------------------+
//         */
//            zone_t *prev_zone = ptr_cast_zone(buf, &zone->prev), *next_zone = ptr_cast_zone(buf, &zone->next);
//            prev_zone->next = zone->next;
//            next_zone->prev = zone->prev;
//        } else panic(BADBRANCH, zone);
//
//        freespace_push(page, PTR_DISTANCE(page, zone),
//                       PTR_DISTANCE(page, ((void *) zone + sizeof(zone_t) + frame->elem_size)));
//        freespace_rebuild(page);
//    }
//
//    return true;
//}

static inline size_t
page_ext_header_sizeof(
    size_t         free_space_cap,
    size_t         lane_reg_cap)
{
    return (free_space_cap * sizeof(range_t) + lane_reg_cap * sizeof(offset_t) + lane_reg_cap * sizeof(lane_id_t));
}

static inline size_t
page_total_header_sizeof(
    size_t         free_space_cap,
    size_t         lane_reg_cap)
{
    return CORE_HDR_SIZE + page_ext_header_sizeof(free_space_cap, lane_reg_cap);
}

static inline void *
seek(
    const page_t * page,
    seek_target    target)
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
            return seek(page, SEEK_LANE_INUSE) + lane_reg(page)->lane_num_limit * sizeof(offset_t);
        case SEEK_PAYLOAD:
            return seek(page, SEEK_LANE_FREELIST) + lane_reg(page)->lane_num_limit * sizeof(lane_id_t);
        default: panic("Unknown seek target '%d'", target);
    }
}

static inline freespace_reg_t *
freespace(
    const page_t * page)
{
    return (freespace_reg_t *) seek(page, SEEK_FREESPACE_REGISTER);
}

static inline void
lane_reg_init(
    page_t *       page,
    size_t         num_lanes)
{
    lane_reg_t *lanes = lane_reg(page);

    lanes->free_list_len = lanes->lane_num_limit = num_lanes;
    for (size_t idx = 0; idx < lanes->free_list_len; idx++) {
        *lane_reg_entry_by_pos(page, idx) = lanes->free_list_len - idx - 1;
    }

    lanes->in_use_num = 0;
    for (size_t idx = 0; idx < lanes->lane_num_limit; idx++) {
        *lane_offsetof(page, idx) = NULL_OFFSET;
    }
}

static inline lane_reg_t *
lane_reg(
    const page_t * page)
{
    return (lane_reg_t *) seek(page, SEEK_LANE_REGISTER);
}

static inline range_t *
freespace_at(
    const page_t * page,
    size_t         pos)
{
    freespace_reg_t *free_space_reg = freespace(page);
    assert (pos < free_space_reg->list_max);
    return seek(page, SEEK_FREESPACE_ENTRIES) + sizeof(range_t) * pos;
}

static inline size_t
freespace_len(
    const page_t * page)
{
    freespace_reg_t *free_space_reg = freespace(page);
    return free_space_reg->list_len;
}

static inline bool
freespace_pop(
    range_t *      range,
    page_t *       page)
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

static inline offset_t *
lane_offsetof(
    const page_t * page,
    lane_id_t      id)
{
    assert (page);
    lane_reg_t *   lanes = lane_reg(page);
    assert (id < lanes->lane_num_limit);
    return seek(page, SEEK_LANE_INUSE) + id * sizeof(offset_t);
}

static inline lane_t *
lane_by_id(
    const page_t * page,
    lane_id_t      id)
{
    assert (page);
    offset_t       offset = *lane_offsetof(page, id);
    void *         data   = read_unsafe(page, offset);
    EXPECT_NONNULL(data, NULL);
    return data;
}

static inline lane_id_t *
lane_reg_entry_by_pos(
    const page_t * page,
    size_t         pos)
{
    lane_reg_t *   lanes = lane_reg(page);
    assert (pos < lanes->lane_num_limit);
    return seek(page, SEEK_LANE_INUSE) + lanes->lane_num_limit * sizeof(offset_t) +
            pos * sizeof(lane_id_t);
}

static inline bool
lane_reg_is_full(
    const page_t * page)
{
    lane_reg_t *   lanes = lane_reg(page);
    return (lanes->free_list_len == 0);
}

static inline lane_id_t
lane_new(
    page_t *       page,
    block_pos      strat,
    size_t         size)
{
    assert (!lane_reg_is_full(page));

    lane_id_t      id         = NULL_LANE_ID;
    range_t        free_range;
    if ((freespace_bind(&free_range, page, LANE_HDR_SIZE, strat))) {
        id = *lane_reg_entry_by_pos(page, --(lane_reg(page))->free_list_len);

        lane_t lane = {
            .first = PAGE_NULL_PTR(),
            .last = PAGE_NULL_PTR(),
            .elem_size = size
        };

        offset_t lane_offset = free_range.begin;
        assert (PTR_DISTANCE(free_range.begin, free_range.end) >= sizeof(lane_t));
        write_unsafe(page, free_range.begin, &lane, sizeof(lane_t));
        lane_reg_link(page, id, lane_offset);
    }
    return id;
}

static inline void
lane_reg_link(
    page_t *       page,
    lane_id_t      id,
    offset_t       lane_offset)
{
    assert(page);
    lane_reg_t *   lanes = lane_reg(page);
    assert(id < lanes->lane_num_limit);
    *(offset_t *)(seek(page, SEEK_LANE_INUSE) + id * sizeof(offset_t)) = lane_offset;
}

static inline range_t *
freespace_new(
    const page_t *   page)
{
    freespace_reg_t *free_space_stack = freespace(page);
    range_t *        entry            = NULL;

    if (free_space_stack->list_len < free_space_stack->list_max) {
        size_t entry_pos = free_space_stack->list_len++;
        entry = freespace_at(page, entry_pos);
    }
    return entry;
}

static inline size_t
freespace_find_first(
    const page_t * page,
    size_t         capacity)
{
    assert (page);
    assert (capacity > 0);

    size_t entry_id_cursor = freespace_len(page);
    while (entry_id_cursor--) {
        range_t *  cursor = freespace_at(page, entry_id_cursor);
        if (OFFSET_DISTANCE(cursor->begin, cursor->end) >= capacity) {
            goto return_position;
        }
    }
    entry_id_cursor = SIZE_MAX;
return_position:

    return entry_id_cursor;
}

static inline size_t
freespace_largest(
    const page_t * page)
{
    assert (page);

    size_t         max_capacity    = 0;
    size_t         entry_id_cursor = freespace_len(page);

    while (entry_id_cursor--) {
        range_t *cursor = freespace_at(page, entry_id_cursor);
        max_capacity = max(max_capacity, OFFSET_DISTANCE(cursor->begin, cursor->end));
    }

    return max_capacity;
}

static inline bool
freespace_bind(
    range_t *      range,
    page_t *       page,
    size_t         size,
    block_pos      strat)
{
    assert(page);
    assert(size > 0);
    assert(range);

    range->begin  = MAX_OFFSET;
    range->end    = NULL_OFFSET;

    const  size_t no_such_entry_id = freespace_len(page);
    size_t entry_id_cursor         = no_such_entry_id, best_entry_id = no_such_entry_id;

    switch (strat) {
        case positioning_first_nomerge: case positioning_first_merge:
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
        if ((strat == positioning_first_merge) ||
            (strat == positioning_smallest_merge) ||
            (strat == positioning_largest_merge)) {
            freespace_rebuild(page);
        }
    }

    bool success = (range->begin < range->end);
    error_if(!success, err_no_free_space);

    return success;
}

static inline range_t
freespace_split(
    page_t *       page,
    size_t         pos,
    size_t         size)
{
    assert (page);

    freespace_reg_t *free_space_reg = freespace(page);
    assert (pos < free_space_reg->list_len);
    range_t * range = freespace_at(page, pos);
    assert (size <= PTR_DISTANCE(range->begin, range->end));

    range_t result = {
        .begin = range->begin,
        .end = range->begin + size
    };

    range->begin = result.end;
    page_approx_freespace_dec(page, size);
    return result;
}

static inline void
freespace_rebuild(
    page_t *       page)
{
    assert (page);
    freespace_cleanup(page);
    freespace_merge(page);
}

static inline void
freespace_cleanup(
    page_t *       page)
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

static inline int
freespace_comp_by_start(
    const void *   lhs,
    const void *   rhs)
{
    offset_t       a = ((range_t *) lhs)->begin;
    offset_t       b = ((range_t *) rhs)->begin;
    return (a == b ? 0 : (a < b) ? - 1 : 1);
}

static inline void
freespace_merge(
    page_t *       page)
{
    assert (page);
    freespace_reg_t * reg = freespace(page);
    size_t            len = freespace_len(page);
    vec_t *        vec = vec_new(sizeof(range_t), len);

    size_t idx = len;
    while (idx--) {
        range_t *range = freespace_at(page, idx);
        vec_pushback(vec, 1, range);
    }

    void *raw_data = vec_data(vec);
    qsort(raw_data, len, sizeof(range_t), freespace_comp_by_start);

    vec_t *stack = vec_new(sizeof(range_t), len);
    vec_pushback(stack, 1, raw_data);

    for (size_t range_idx = 0; range_idx < len; range_idx++) {
        range_t *current_range = (range_t *)(raw_data + range_idx * sizeof(range_t));
        range_t *stack_top = vec_peek(stack);
        if (range_do_overlap(current_range, stack_top)) {
            stack_top->end = max(stack_top->end, current_range->end);
        } else vec_pushback(stack, 1, current_range);
    }

    reg->list_len = stack->num_elements;
    memcpy(freespace_at(page, 0), vec_data(stack), stack->num_elements * sizeof(range_t));

    vec_free(vec);
    vec_free(stack);
}

static inline lane_id_t
lane_reg_scan(
    const page_t * page,
    lane_state     state)
{
    static size_t  lane_id;
    static page_t *last_page = NULL;
    if (page != NULL) {
        last_page = (page_t *) page;
        lane_id = lane_reg(last_page)->lane_num_limit;
    }

    while (lane_id--) {
        offset_t lane_offset = *lane_offsetof(last_page, lane_id);
        if ((state == LANE_STATE_INUSE && lane_offset != NULL_OFFSET) ||
            (state == LANE_STATE_FREE  && lane_offset == NULL_OFFSET))
            return lane_id;
    }

    return NULL_LANE_ID;
}

static inline bool
ptr_is_null(
    const in_page_ptr *ptr)
{
    assert(ptr);
    return (ptr->offset == NULL_OFFSET);
}

static inline bool
ptr_has_scope(
    const in_page_ptr *ptr,
    page_ptr_type      type)
{
    assert (ptr);
    switch (type) {
        case PAGE_PTR_FAR:  return (ptr->is_far_ptr);
        case PAGE_PTR_NEAR: return (!ptr->is_far_ptr);
        default:
            error(err_internal);
            return false;
    }
}

static inline void
ptr_make(
    in_page_ptr *  ptr,
    page_t *       page,
    page_ptr_type  type,
    ptr_target     target,
    offset_t       offset)
{
    assert(ptr);
    assert(page);
    assert(offset != NULL_OFFSET);

    ptr->page_id    = page->header.id;
    ptr->is_far_ptr = (type == PAGE_PTR_FAR);
    ptr->offset     = offset;

    switch (target) {
        case TARGET_LANE:
            ptr->target_type_bit_0 = ptr->target_type_bit_1 = 0;
            break;
        case TARGET_ZONE:
            ptr->target_type_bit_0 = 0;
            ptr->target_type_bit_1 = 1;
            break;
        case TARGET_USERDATA:
            ptr->target_type_bit_0 = 1;
            ptr->target_type_bit_1 = 0;
            break;
        case TARGET_CORRUPTED:
            ptr->target_type_bit_0 = 1;
            ptr->target_type_bit_1 = 1;
            break;
        default:
            panic("Unknown pointer target type '%d'", target);
    }
}

static inline
ptr_target ptr_typeof(
    const in_page_ptr *ptr)
{
    assert (ptr);
    if (ptr->target_type_bit_0 == 0) {
        if (ptr->target_type_bit_1 == 0) {
            return TARGET_LANE;
        } else {
            return TARGET_ZONE;
        }
    } else {
        if (ptr->target_type_bit_1 == 0) {
            return TARGET_USERDATA;
        } else {
            return TARGET_CORRUPTED;
        }
    }
}

static inline void *
ptr_deref(
    anti_buf_t *       buf,
    const in_page_ptr *ptr)
{
    panic_if((buf == NULL), BADARGNULL, to_string(buf));
    panic_if((ptr == NULL), BADARGNULL, to_string(ptr));
    panic_if((buf->page_anticache.hot_store == NULL), UNEXPECTED, "page register hash table is null");
    anticache_guarantee_page(buf, ptr->page_id);
    void *page_base_ptr = hotstore_get_unsafe(buf, ptr->page_id);
    panic_if((page_base_ptr == NULL), UNEXPECTED, "page base pointer is not allowed to be null");
    return (page_base_ptr + ptr->offset);
}

static inline zone_t *
ptr_cast_zone(
    anti_buf_t *       buf,
    const in_page_ptr *ptr)
{
    panic_if(ptr_typeof(ptr) != TARGET_ZONE, BADCAST, ptr);
    void *data = ptr_deref(buf, ptr);
    return (zone_t *) data;
}

//static inline lane_t *
//ptr_cast_frame(
//    anti_buf_t *buffer_manager,
//    const in_page_ptr *ptr)
//{
//    panic_if(ptr_typeof(ptr) != TARGET_LANE, BADCAST, ptr);
//    void *data = ptr_deref(buffer_manager, ptr);
//    return (lane_t *) data;
//}

//static inline lane_t *
//lane_by_zone(
//        anti_buf_t *manager,
//        const zone_t *zone)
//{
//    while (!ptr_is_null(&zone->prev) && ptr_typeof(&zone->prev) != TARGET_LANE) {
//        zone = ptr_cast_zone(manager, &zone->prev);
//        panic_if(!ptr_has_scope(&zone->prev, type_near_ptr), BADHURRY, "Traversing over page boundaries "
//                "not implemented!");
//    }
//    return ptr_cast_frame(manager, &zone->prev);
//}

static inline void
write_unsafe(
    page_t *       page,
    offset_t       offset,
    const void *   data,
    size_t         size)
{
    assert (page);
    assert (offset >= PTR_DISTANCE(page, seek(page, SEEK_PAYLOAD)));
    assert (data);
    assert (size > 0);

    void *         base = (void *) page;
    memcpy(base + offset, data, size);
}

static inline void *
read_unsafe(
    const page_t * page,
    offset_t       offset)
{
    assert (page);
    assert (offset >= PTR_DISTANCE(page, seek(page, SEEK_PAYLOAD)));
    return (offset != NULL_OFFSET ? (void *) page + offset : NULL);
}

static inline bool
range_do_overlap(
    range_t *      lhs,
    range_t *      rhs)
{
    return (lhs->end >= rhs->begin && lhs->begin <= rhs->begin) ||
           (rhs->end >= lhs->begin && rhs->begin <= lhs->begin);
}

static inline void
page_approx_freespace_inc(
    page_t *       page,
    size_t         size)
{
    assert (page);
    page->header.freespace += size;
}

static inline void
page_approx_freespace_dec(
    page_t *       page,
    size_t         size)
{
    assert (page);
    page->header.freespace -= size;
}

static inline size_t
page_approx_freespace(
    const page_t * page)
{
    return page->header.freespace;
}

static inline bool
freespace_push(
    page_t *      page,
    offset_t      begin,
    offset_t      end)
{
    range_t *     entry = NULL;

    if ((entry = freespace_new(page))) {
        EXPECT_NONNULL(entry, false);
        entry->begin = begin;
        entry->end = end;
    }
    panic_if((entry == NULL), UNEXPECTED, "Free store capacity exceeded");
    page_approx_freespace_inc(page, OFFSET_DISTANCE(begin, end));
    return (entry != NULL);
}

static inline void
buf_init(
    anti_buf_t *   buf)
{
    buf->page_anticache.hot_store = hash_table_new(
            &(hash_function_t) {.capture = NULL, .hash_code = hash_code_jen},
            sizeof(page_id_t), sizeof(void *), HOTSTORE_INITCAP,
            HOTSTORE_GROW_FACTOR, HOTSTORE_MAX_FILL_FAC);
    buf->config = (buf_config_t) {
            .free_space_reg_capacity = 100,
            .lane_reg_capacity = 100
    };
    conf_get_size_t(&buf->config.hotstore_size_limit,   CONF_SETTING_SWAP_BUFFER_HOTSTORE_LIM,  8589934592);
    conf_get_size_t(&buf->config.ram_page_size_default, CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_DEF,  858993459);
    conf_get_size_t(&buf->config.ram_page_size_max,     CONF_SETTING_SWAP_BUFFER_PAGE_SIZE_MAX, 3221225472);

    REQUIRE((buf->config.ram_page_size_default > 0), "Bad default page size");
    REQUIRE((buf->config.hotstore_size_limit > 0),  "Bad hot store limit");
    REQUIRE((buf->config.free_space_reg_capacity > 0), "Bad capacity of free space register");
    REQUIRE((buf->config.lane_reg_capacity > 0), "Bad capacity of lane register");
    REQUIRE((buf->config.ram_page_size_max < buf->config.hotstore_size_limit), "miss-configured.");
    REQUIRE((buf->config.ram_page_size_default < buf->config.ram_page_size_max), "miss-configured.");

    panic_if((buf->config.ram_page_size_max > 0.5 * buf->config.hotstore_size_limit),
             BADPAGESIZE, (size_t) buf->config.ram_page_size_max, (size_t) buf->config.hotstore_size_limit);


    GS_REQUIRE_NONNULL(buf->page_anticache.hot_store);
}

static inline page_t *
generic_store_get(
    anti_buf_t *     buf,
    size_t           size,
    const page_id_t *(*iterate)(anti_buf_t *, const page_id_t *),
    page_t *         (*fetch)(anti_buf_t *, page_id_t),
    size_t           (*get_capacity)(page_t *, free_space_get_strat))
{
    const page_id_t *cursor = NULL;
    page_t *         page = NULL;

    while ((cursor = iterate(buf, cursor)) != NULL) {
        page = fetch(buf, *cursor);
        size_t quick_free = get_capacity(page, FREE_SPACE_GET_APPROX);
        if (quick_free >= size) {
            size_t exact_free = get_capacity(page, FREES_PACE_GET_EXACT);
            if (exact_free >= size)
                return page;
        }
    }

    return NULL;
}

static inline page_t *
anticache_page_by_freesize(
    anti_buf_t *   buf,
    size_t         size,
    page_t *       favored)
{
    page_t *       page = NULL;

    if ((favored != NULL) && (page_get_free_space(favored, FREE_SPACE_GET_APPROX) >= size) &&
        (page_get_free_space(favored, FREES_PACE_GET_EXACT) >= size)) {
        page = favored;
    } else {
        if (!hotstore_is_empty(buf)) {
            page = generic_store_get(buf, size, hotstore_iterate, hotstore_get_unsafe,
                                     page_get_free_space);
        }
        if (page == NULL && !coldstore_is_empty(buf)) {
            page = generic_store_get(buf, size, coldstore_iterate, coldstore_fetch,
                                     page_get_free_space);
            anticache_add_page_safe(buf, page);
        }
        if (page == NULL) {
            page = anticache_create_page_safe(buf, size);
        }
    }

    return page;
}

static inline void
anticache_pin(
    anti_buf_t *   buf,
    page_t *       page)
{
    warn(NOTIMPLEMENTED, to_string(anticache_pin));   // TODO: Implement
}

static inline void
anticache_unpin(
    anti_buf_t *   buf,
    page_t *       page)
{
    warn(NOTIMPLEMENTED, to_string(anticache_unpin));   // TODO: Implement
}

static inline lane_id_t
anticache_create_lane(
    anti_buf_t *   buf,
    page_t *       page,
    block_pos      strat,
    size_t         size
) {
    GS_REQUIRE_NONNULL(buf);
    GS_REQUIRE_NONNULL(page);
    REQUIRE((size > 0), "size must be non-zero");
    panic_if(!(hotstore_has_unsafe(buf, page->header.id)), BADHOTSTOREOBJ, page->header.id);
    lane_handle_t *lane_id = lane_create(page, strat, size);            // TODO: just return lane_id here instead of lane handle
    panic_if((lane_id == NULL), UNEXPECTED, "Lane handle is null");
    lane_id_t result = lane_id->lane_id;
    free (lane_id);
    return result;
}