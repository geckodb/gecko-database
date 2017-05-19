#include <storage/slotted_page.h>
#include <require.h>
#include <assert.h>
#include <error.h>
#include <storage/schema.h>

static bool _init_pages(slotted_page_file_t *file, unsigned page_size, unsigned num_pages);
static bool _free_page(void *capture, void *from, void *to);
static addressing_mode _get_mode(schema_t *schema);

slotted_page_file_t *slotted_page_file_create(schema_t *schema, unsigned page_size, unsigned num_pages)
{
    slotted_page_file_t *file = NULL;
    if (require_non_null(schema) && require_non_null(schema->attributes) &&
        require_non_zero(page_size) && require_non_zero(num_pages) &&
        ((file = require_malloc(sizeof(slotted_page_file_t))))) {
        file->schema = schema_cpy(schema);
        file->page_size = page_size;
        file->mode = _get_mode(schema);
        if (!_init_pages(file, page_size, num_pages)) {
            return NULL;
        }
    }
    return file;
}

bool slotted_page_file_free(slotted_page_file_t *file)
{
    return require_non_null(file) && schema_free(file->schema) && vector_free(file->free_page_ids) &&
           vector_free_ex(file->slotted_pages, NULL, _free_page);
}

void _setup_slot(page_slot_t *begin, page_slot_t *end)
{
    page_slot_t empty_slot = {
        .header.field_in_byte = 0,
        .header.is_forward_tuple_id = false,
    };
    for (page_slot_t *it = begin; it < end; it++) {
        empty_slot.payload = (it + sizeof(page_slot_t));
        memcpy(it, &empty_slot, sizeof(page_slot_t));
    }
}

void _setup_page(slotted_page_t *page, unsigned page_id, unsigned page_size) {
    atomic_store(&page->header.is_locked, false);
    page->header.num_contained_tuples = 0;
    page->header.page_id = page_id;
    page->header.tuple_size = 0;
    assert (page_size > sizeof(page_slot_t));
    page->slots_begin = (page_slot_t *)(page + sizeof(slotted_page_t));
    page->slots_end   = (page_slot_t *)(page->slots_begin + page_size);
    _setup_slot(page->slots_begin, page->slots_end);
}

bool _free_page(void *capture, void *from, void *to)
{
    if (require_non_null(capture) && require_non_null(from) && require_non_null(to) && require_less_than(from, to)) {
        slotted_page_t **begin = (slotted_page_t **) from;
        slotted_page_t **end = (slotted_page_t **) to;
        for (slotted_page_t **page_ptr = begin; page_ptr < end; page_ptr++) {
            free (*page_ptr);
        }
        return true;
    }
    return false;
}

bool _alloc_page(void *capture, void *from, void *to)
{
    if (require_non_null(capture) && require_non_null(from) && require_non_null(to) && require_less_than(from, to)) {
        slotted_page_t **begin = (slotted_page_t **) from;
        slotted_page_t **end = (slotted_page_t **) to;
        unsigned page_size = *(unsigned *) capture;
        for (slotted_page_t **page_ptr = begin; page_ptr < end; page_ptr++) {
            unsigned page_id = (page_ptr - begin);
            if ((*page_ptr = (slotted_page_t *) require_malloc(sizeof(slotted_page_t) + page_size))) {
                _setup_page(*page_ptr, page_id, page_size);
            }
        }
        return true;
    } else return false;
}

bool _init_pages(slotted_page_file_t *file, unsigned page_size, unsigned num_pages)
{
    assert (file != NULL && page_size > 0 && num_pages > 0);
    if ((file->slotted_pages = vector_create(sizeof(slotted_page_t *), num_pages))) {
        unsigned page_size = file->page_size;
        if (vector_resize(file->slotted_pages, num_pages) &&
            vector_foreach(file->slotted_pages, &page_size, _alloc_page) &&
            (file->free_page_ids = vector_create(sizeof(unsigned), num_pages))) {
            for (unsigned page_id = 0; page_id < num_pages; page_id++) {
                vector_add(file->free_page_ids, 1, &page_id);
            }
        }
    }
    return false;
}

static addressing_mode _get_mode(schema_t *schema)
{
    return schema_is_fixed_size(schema) ? mode_fixed_size : mode_variable_size;
}