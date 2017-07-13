#pragma once

typedef struct
{
    tpch_identifier_t N_NATIONKEY;
    tpch_text_t       N_NAME;
    tpch_identifier_t N_REGIONKEY;
    tpch_text_t       N_COMMENT;
} tpch_nation_tuple_t;

typedef struct
{
    char             *N_NATIONKEY;
    char             *N_NAME;
    char             *N_REGIONKEY;
    char             *N_COMMENT;
} tpch_nation_tuple_str_t;