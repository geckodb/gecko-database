#include <frag.h>
#include <frags/frag_host_vm.h>
#include <frag_printer.h>
#include <schema.h>

void gs_checksum_nsm(schema_t *tab, const void *tuplets, size_t ntuplets)
{
    panic(NOTIMPLEMENTED, "this")
    /*
    size_t num_attr = tab->attr->num_elements;
    attr_t *attr = (attr_t *) tab->attr->data;
    size_t tuple_size = get_tuple_size(tab);
    checksum_context_t column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        const void *cursor = tuplets;
        panic(NOTIMPLEMENTED, "this")
        size_t field_offs = get_tuple_size(tab);
        size_t field_size = get_field_size(attr, attr_idx);
        begin_checksum(&column_checksum);
        for (size_t tuple_idx = 0; tuple_idx < num_attr; tuple_idx++) {
            update_checksum(&column_checksum, cursor + field_offs, cursor + field_offs + field_size);
            cursor += tuple_size;
        }
        end_checksum(attr[attr_idx].checksum, &column_checksum);
    }*/
}

void gs_checksum_dms(schema_t *tab, const void *tuplets, size_t ntuplets)
{
    panic(NOTIMPLEMENTED, "this")
    /*size_t num_attr = tab->attr->num_elements;
    attr_t *attr = (attr_t *) tab->attr->data;
    checksum_context_t column_checksum;

    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
        size_t field_size = get_field_size(attr, attr_idx);
        size_t chunk_size = ntuplets * field_size;
        begin_checksum(&column_checksum);
        update_checksum(&column_checksum, tuplets, tuplets + chunk_size);
        end_checksum(attr[attr_idx].checksum, &column_checksum);
        tuplets += chunk_size;
    }*/
}

void gs_checksum_begin(checksum_context_t *context)
{
    MD5_Init (context);
}

void gs_checksum_update(checksum_context_t *context, const void *begin, const void *end) {
    MD5_Update (context, begin, (end - begin));
}

void gs_checksum_end(unsigned char *checksum_out, checksum_context_t *context)
{
    MD5_Final (checksum_out,context);
}

size_t find_type_match(enum frag_impl_type_t type)
{
    size_t len = ARRAY_LEN_OF(frag_type_pool);
    for (size_t i = 0; i < len; i++) {
        if (frag_type_pool[i].binding == type) {
            return i;
        }
    }
    panic(BADFRAGTYPE, type);
}


frag_t *gs_fragment_alloc(schema_t *schema, size_t tuplet_capacity, enum frag_impl_type_t type)
{
    REQUIRE((tuplet_capacity > 0), "capacity of tuplets must be non zero");

    frag_t *result = frag_type_pool[find_type_match(type)]._create(schema, tuplet_capacity);
    result->impl_type = type;

    panic_if((result->_dispose == NULL), NOTIMPLEMENTED, "frag_t::dispose");
    panic_if((result->_scan == NULL), NOTIMPLEMENTED, "frag_t::scan");
    panic_if((result->_open == NULL), NOTIMPLEMENTED, "frag_t::open");
    panic_if((result->_insert == NULL), NOTIMPLEMENTED, "frag_t::this_query");
    return result;
}

void gs_frag_insert(struct tuplet_t *out, frag_t *frag, size_t ntuplets)
{
    assert (frag);
    assert (ntuplets > 0);
    assert (frag->_insert);
    struct tuplet_t *result = frag->_insert(frag, ntuplets);
    REQUIRE_NONNULL(result)
    REQUIRE_NONNULL(result->fragment)
    REQUIRE_NONNULL(result->attr_base)
    if (out != NULL) {
        memcpy(out, result, sizeof(struct tuplet_t));
    }
    free (result);
}

void gs_frag_print(FILE *file, frag_t *frag, size_t row_offset, size_t limit)
{
    gs_frag_print_ex(file, FPTT_CONSOLE_PRINTER, frag, row_offset, limit);
}

void gs_frag_print_ex(FILE *file, enum frag_printer_type_tag type, frag_t *frag, size_t row_offset, size_t limit)
{
    gs_frag_printer_print(file, type, frag, row_offset, limit);
}

void gs_fragment_free(frag_t *frag)
{
    assert(frag);
    frag->_dispose(frag);
}

const char *gs_frag_str(enum frag_impl_type_t type)
{
    switch (type) {
        case FIT_HOST_NSM_VM: return "host/vm nsm";
        case FIT_HOST_DSM_VM: return "host/vm dsm";
        default: panic("Unknown fragment implementation type '%d'", type);
    }
}

size_t gs_fragment_num_of_attributes(const frag_t *frag)
{
    assert (frag);
    return gs_schema_num_attributes(frag->schema);
}

size_t gs_fragment_num_of_tuplets(const frag_t *frag)
{
    assert (frag);
    return frag->ntuplets;
}

schema_t *gs_frag_get_schema(const frag_t *frag)
{
    assert(frag);
    return frag->schema;
}

enum field_type gs_fragment_get_field_type(const frag_t *frag, attr_id_t id)
{
    assert (frag);
    return gs_schema_attr_type(frag->schema, id);
}
