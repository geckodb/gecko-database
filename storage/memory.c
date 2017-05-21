#include <storage/memory.h>
#include <require.h>
#include <assert.h>
#include <limits.h>
#include <containers/vector.h>

#define INVALID_SCHEMA_ID UINT_MAX
#define NO_SLOT_ID UINT_MAX

page_file_t *page_file_create()
{
    page_file_t *file = NULL;
    if ((file = require_malloc(sizeof(page_file_t))) && ((file->pages = vector_create(sizeof(page_t*), 25))) &&
        ((file->schema_register.free_list_schema_ids = vector_create(sizeof(schema_id_t), 25))) &&
        ((file->schema_register.schemas = vector_create(sizeof(schema_t), 25)))) { };
    return file;
}

bool _free_file_pages(void *capture, void *begin, void *end)
{
    // TODO
    return true;
}

bool _free_file_schemas(void *capture, void *begin, void *end)
{
    // TODO
    return true;
}

bool page_file_free(page_file_t *file)
{
    if (require_non_null(file)) {
        if (require_non_null(file->pages)) {
            vector_free_ex(file->pages, NULL, _free_file_pages);
        } else return false;
        if (require_non_null(file->schema_register.schemas)) {
            vector_free_ex(file->schema_register.schemas, NULL, _free_file_schemas);
        } else return false;
        if (require_non_null(file->schema_register.free_list_schema_ids)) {
            vector_free(file->schema_register.free_list_schema_ids);
        } else return false;
        free (file);
        return true;
    }
    return false;
}

bool _valid_file(page_file_t *file)
{
    return (require_non_null(file) && require_non_null(file->pages) &&
            require_non_null(file->schema_register.free_list_schema_ids) &&
            require_non_null(file->schema_register.schemas));
}

bool _add_page(page_file_t *file, page_t *page)
{
    return vector_add(file->pages, 1, page);
}

page_t *page_create(page_file_t *file, size_t page_size)
{
    page_t *page = NULL;
    if (_valid_file(file) &&
        (require_constraint_size_t(page_size, constraint_greater_equal, sizeof(fixed_tuple_slot_t))) &&
        ((page = require_malloc(sizeof(page_t)))) &&
        ((page->data_begin = require_malloc(page_size))) &&
        ((page->slot_free_list = vector_create(sizeof(unsigned), 100)))) {
        page->var_data_end = page->data_begin;
        page->first_tuple = page->data_end = page->data_begin + page_size;
        page->file = file;
        page->num_tuples = 0;
        _add_page(file, page);
    }
    return page;
}

bool _file_remove_page(page_t *page)
{
    // TODO
    return true;
}

bool _valid_page(page_t *page)
{
    return (require_non_null(page) && require_non_null(page->slot_free_list) && require_non_null(page->data_begin) &&
            require_non_null(page->data_end) && require_non_null(page->first_tuple));
}


bool page_free(page_t *page)
{
    return (_valid_page(page) && _file_remove_page(page));
}

schema_t *_convert_to_fixed(const schema_t *schema, const void *data)
{
    assert (schema != NULL && data != NULL);

    if (schema_is_fixed_size(schema)) {
        return schema_cpy(schema);
    } else {
        size_t num_attr = schema_num_attributes(schema);
        schema_t *fixed_schema = schema_create(num_attr);
        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
            data_type type = schema_get(schema, attr_idx);
            const char *attr_name = schema_get_attr_name(schema, attr_idx);
            attribute_flags_t attr_flags_t = schema_get_attr_flags(schema, attr_idx);
            attribute_flags attr_flags = attribute_flags_t_to_flag(attr_flags_t);
            attribute_t *fixed_schema_attr = NULL;
            if (type_is_fixed_size(type)) {
                fixed_schema_attr = attribute_create(type, 1, attr_name, attr_flags);
                data += type_sizeof(type);
            } else {
                attribute_t *attr_len = attribute_create(type_internal_strlen, 1, "_length_", attribute_nonnull);
                schema_add(fixed_schema, attr_len);
                attribute_free(attr_len);
                fixed_schema_attr = attribute_create(type_internal_pointer, 1, attr_name, attr_flags);
                size_t varseq_length = strlen((const char *) data);
                fixed_schema->varseq_length += varseq_length;
                data += varseq_length + 1;
            }
            schema_add(fixed_schema, fixed_schema_attr);
        }
        return fixed_schema;
    }
}

schema_id_t _find_schema_in_register(vector_t *schemas, const schema_t *needle)
{
    assert (schemas != NULL && needle != NULL);

    schema_id_t schema_id = INVALID_SCHEMA_ID;
    const schema_t *all_schemas_begin = vector_get(schemas);
    const schema_t *all_schemas_end = all_schemas_begin + schemas->num_elements;
    for (const schema_t *it = all_schemas_begin; it < all_schemas_end; it++) {
        if (schema_comp(needle, it)) {
            return (it - all_schemas_begin);
        }
    }
    return schema_id;
}

schema_id_t _add_schema_to_register(vector_t *schemas, const schema_t *needle)
{
    assert (schemas != NULL && needle != NULL);
    schema_id_t id = schemas->num_elements;
    bool result = vector_add(schemas, 1, needle);
    assert (result);
    return id;
}

schema_id_t _get_schema_id(page_file_t *file, const schema_t *fixed_schema)
{
    assert (file != NULL && fixed_schema != NULL);
    schema_id_t schema_id;
    if ((schema_id = _find_schema_in_register(file->schema_register.schemas, fixed_schema)) == INVALID_SCHEMA_ID) {
        return _add_schema_to_register(file->schema_register.schemas, fixed_schema);
    }  else return schema_id;
}

size_t _check_space(page_t *page, const schema_t *fixed_schema)
{
    size_t tuple_size = schema_get_tuple_size(fixed_schema);
    size_t varseq_size = schema_get_varseq_size(fixed_schema);
    return  (page->var_data_end + varseq_size < page->first_tuple - tuple_size - sizeof(fixed_tuple_slot_t)) ?
            tuple_size : 0;
}

void _write_varseq_data(page_t *page, const schema_t *fixed_schema, const void *data)
{
/*    const void *varseq = schema_seek_next_varseq(fixed_schema, data);
    while (varseq) {
        size_t varseq_len = strlen(varseq);
        memcpy(page->var_data_end, varseq, varseq_len);
        page->var_data_end += varseq_len;
        const void *varseq = schema_seek_next_varseq(NULL, NULL);
    }*/
}

unsigned _write_tuple_data(page_t *page, schema_id_t schema_id, size_t tuple_size, const void *data) {
    assert (schema_id < 16384); /* 2^14 bits for schema id storage, makes an upper bound of 16384 */
    void *write_offset = page->first_tuple - tuple_size - sizeof(fixed_tuple_slot_t);
    fixed_tuple_slot_t tuple = {
        .header.is_forward_tuple = false,
        .header.is_free = false,
        .header.schema_id = schema_id
    };
    *((fixed_tuple_slot_t *) write_offset) = tuple;
    memcpy((write_offset + sizeof(fixed_tuple_slot_t)), data, tuple_size);
    page->num_tuples++;
    return page->num_tuples++;
}

unsigned _write_to_page(page_t *page, const schema_t *fixed_schema, schema_id_t schema_id, const void *data)
{
    unsigned slot_id = NO_SLOT_ID;
    size_t tuple_size = 0;
    if ((tuple_size = _check_space(page, fixed_schema))) {
        _write_varseq_data(page, fixed_schema, data);
        slot_id = _write_tuple_data(page, schema_id, tuple_size, data);
    }
    return slot_id;
}

paged_tuple_handle_t page_write_tuple(page_t *page, const schema_t *schema, const void *data)
{
    paged_tuple_handle_t tuple_id = {
        .page = NULL,
        .slot = NULL_TUPLE_ID
    };

    if (_valid_page(page) && require_non_null(schema) && require_non_null(data)) {
        schema_print(stdout, schema);
        fprintf(stdout, "\n");
        fflush(stdout);
        schema_t *fixed_schema = _convert_to_fixed(schema, data);
        schema_print(stdout, fixed_schema);
        fflush(stdout);
        schema_id_t fixed_schema_id = _get_schema_id(page->file, fixed_schema);
        unsigned slot_id;
        if ((slot_id = _write_to_page(page, fixed_schema, fixed_schema_id, data)) != NO_SLOT_ID) {
            tuple_id.page = page;
            tuple_id.slot = slot_id;
        }
    }
    return tuple_id;
}