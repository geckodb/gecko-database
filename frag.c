#include <frag.h>
#include <frags/frag_nsm.h>
#include <frags/frag_dsm.h>
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


frag_t *gs_fragment_alloc(schema_t *schema, size_t tuplet_capacity, enum tuplet_format format)
{
    require((tuplet_capacity > 0), "capacity of tuplets must be non zero");

    frag_t *result;
    switch (format) {
        case TF_NSM:
            result = gs_fragment_nsm_alloc(schema, tuplet_capacity);
            break;
        case TF_DSM:
            result = gs_fragment_dsm_alloc(schema, tuplet_capacity);
            break;
        default:
            panic(BADARG, " tuplet format");
    }
    panic_if((result->_dispose == NULL), NOTIMPLEMENTED, "frag_t::dispose");
    panic_if((result->_scan == NULL), NOTIMPLEMENTED, "frag_t::scan");
    panic_if((result->_open == NULL), NOTIMPLEMENTED, "frag_t::open");
    panic_if((result->_insert == NULL), NOTIMPLEMENTED, "frag_t::insert");
    return result;
}

struct tuplet_t *gs_fragment_insert(frag_t *frag, size_t ntuplets)
{
    assert (frag);
    assert (ntuplets > 0);
    assert (frag->_insert);
    return frag->_insert(frag, ntuplets);
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
    assert(frag->tuplet_data);
    free (frag->tuplet_data);
    free (frag);
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

schema_t *gs_fragment_get_schema(const frag_t *frag)
{
    assert(frag);
    return frag->schema;
}

enum field_type gs_fragment_get_field_type(const frag_t *frag, attr_id_t id)
{
    assert (frag);
    return gs_schema_attr_type(frag->schema, id);
}
