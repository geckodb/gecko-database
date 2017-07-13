#pragma once

typedef struct
{
    tpch_identifier_t R_REGIONKEY;
    tpch_text_t       R_NAME;
    tpch_text_t       R_COMMENT;
} tpch_region_tuple_t;

typedef struct
{
    char             *R_REGIONKEY;
    char             *R_NAME;
    char             *R_COMMENT;
} tpch_region_tuple_str_t;