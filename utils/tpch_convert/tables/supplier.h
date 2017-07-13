#pragma once

typedef struct
{
    tpch_identifier_t S_SUPPKEY;
    tpch_text_t       S_NAME;
    tpch_text_t       S_ADDRESS;
    tpch_identifier_t S_NATIONKEY;
    tpch_text_t       S_PHONE;
    tpch_decimal_t    S_ACCTBAL;
    tpch_text_t       S_COMMENT;
} tpch_supplier_tuple_t;

typedef struct
{
    char             *S_SUPPKEY;
    char             *S_NAME;
    char             *S_ADDRESS;
    char             *S_NATIONKEY;
    char             *S_PHONE;
    char             *S_ACCTBAL;
    char             *S_COMMENT;
} tpch_supplier_tuple_str_t;