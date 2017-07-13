#pragma once

typedef struct
{
    tpch_identifier_t P_PARTKEY;
    tpch_text_t       P_NAME;
    tpch_text_t       P_MFGR;
    tpch_text_t       P_BRAND;
    tpch_text_t       P_TYPE;
    tpch_integer_t    P_SIZE;
    tpch_text_t       P_CONTAINER;
    tpch_decimal_t    P_RETAILPRICE;
    tpch_text_t       P_COMMENT;
} tpch_part_tuple_t;

typedef struct
{
    char             *P_PARTKEY;
    char             *P_NAME;
    char             *P_MFGR;
    char             *P_BRAND;
    char             *P_TYPE;
    char             *P_SIZE;
    char             *P_CONTAINER;
    char             *P_RETAILPRICE;
    char             *P_COMMENT;
} tpch_part_tuple_str_t;