#include <frag.h>
#include <frags/frag_nsm.h>
#include <frags/frag_dsm.h>

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


fragment_t *gs_fragment_alloc(schema_t *schema, size_t tuplet_capacity, enum tuplet_format format)
{
    require((tuplet_capacity > 0), "capacity of tuplets must be non zero");

    fragment_t *result;
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
    panic_if((result->_dispose == NULL), NOTIMPLEMENTED, "fragment_t::dispose");
    panic_if((result->_scan == NULL), NOTIMPLEMENTED, "fragment_t::scan");
    panic_if((result->_open == NULL), NOTIMPLEMENTED, "fragment_t::open");
    panic_if((result->_insert == NULL), NOTIMPLEMENTED, "fragment_t::insert");
    return result;
}

struct tuplet_t *gs_fragment_insert(fragment_t *frag, size_t ntuplets)
{
    assert (frag);
    assert (ntuplets > 0);
    assert (frag->_insert);
    return frag->_insert(frag, ntuplets);
}

void gs_fragment_free(fragment_t *frag)
{
    assert(frag);
    assert(frag->tuplet_data);
    free (frag->tuplet_data);
    free (frag);
}

size_t gs_sizeof(enum field_type type)
{
    switch (type) {
        case FT_CHAR:
            return sizeof(CHAR);
        case FT_BOOL:
            return sizeof(BOOL);
        case FT_INT8:
            return sizeof(INT8);
        case FT_UINT8:
            return sizeof(UINT8);
        case FT_INT16:
            return sizeof(INT16);
        case FT_UINT16:
            return sizeof(UINT16);
        case FT_INT32:
            return sizeof(INT32);
        case FT_UINT32:
            return sizeof(UINT32);
        case FT_FLOAT32:
            return sizeof(FLOAT32);
        case FT_INT64:
            return sizeof(INT64);
        case FT_UINT64:
            return sizeof(UINT64);
        case FT_FLOAT64:
            return sizeof(FLOAT64);
        default:
            perror ("Unknown type");
            abort();
    }
}
