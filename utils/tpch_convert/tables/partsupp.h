#pragma once

typedef struct
{
    tpch_identifier_t PS_PARTKEY;
    tpch_identifier_t PS_SUPPKEY;
    tpch_integer_t    PS_AVAILQTY;
    tpch_decimal_t    PS_SUPPLYCOST;
    tpch_text_t       PS_COMMENT;
} tpch_partsupp_tuple_t;

typedef struct
{
    char             *PS_PARTKEY;
    char             *PS_SUPPKEY;
    char             *PS_AVAILQTY;
    char             *PS_SUPPLYCOST;
    char             *PS_COMMENT;
} tpch_partsupp_tuple_str_t;