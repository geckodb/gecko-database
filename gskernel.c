
void *gs_update(void *dst, SCHEMA *frag, ATTR_ID attr_id, void *src)
{
    assert (dst && frag && src);



    switch (gs_schema_attr_by_id(attr_id)->type) {
        case FT_BOOL:    return gs_insert_bool(dst, frag, attr_id,    (const BOOL *) src);
        case FT_INT8:    return gs_insert_int8(dst, frag, attr_id,    (const INT8 *) src);
        case FT_INT16:   return gs_insert_int16(dst, frag, attr_id,   (const INT16 *) src);
        case FT_INT32:   return gs_insert_int32(dst, frag, attr_id,   (const INT32 *) src);
        case FT_INT64:   return gs_insert_int64(dst, frag, attr_id,   (const INT64 *) src);
        case FT_UINT8:   return gs_insert_uint8(dst, frag, attr_id,   (const UINT8 *) src);
        case FT_UINT16:  return gs_insert_uint16(dst, frag, attr_id,  (const UINT16 *) src);
        case FT_UINT32:  return gs_insert_uint32(dst, frag, attr_id,  (const UINT32 *) src);
        case FT_UINT64:  return gs_insert_uint64(dst, frag, attr_id,  (const UINT64 *) src);
        case FT_FLOAT32: return gs_insert_float32(dst, frag, attr_id, (const FLOAT32 *) src);
        case FT_FLOAT64: return gs_insert_float64(dst, frag, attr_id, (const FLOAT64 *) src);
        case FT_CHAR:    return gs_insert_string(dst, frag, attr_id,  (const CHAR *) src);
    }
}

const char *gs_type_str(enum field_type t)
{
    switch (t) {
        case FT_BOOL:    return "bool";
        case FT_INT8:    return "int8";
        case FT_INT16:   return "int16";
        case FT_INT32:   return "int32";
        case FT_INT64:   return "int64";
        case FT_UINT8:   return "uint8";
        case FT_UINT16:  return "uint16";
        case FT_UINT32:  return "uint32";
        case FT_UINT64:  return "uint64";
        case FT_FLOAT32: return "float32";
        case FT_FLOAT64: return "float64";
        case FT_CHAR:  return "string";
        default: return "(unknown)";
    }
}





