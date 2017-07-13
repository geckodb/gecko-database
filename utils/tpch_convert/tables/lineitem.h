#pragma once

typedef struct
{
    tpch_identifier_t L_ORDERKEY;
    tpch_identifier_t L_PARTKEY;
    tpch_identifier_t L_SUPPKEY;
    tpch_integer_t    L_LINENUMBER;
    tpch_decimal_t    L_QUANTITY;
    tpch_decimal_t    L_EXTENDEDPRICE;
    tpch_decimal_t    L_DISCOUNT;
    tpch_decimal_t    L_TAX;
    tpch_text_t       L_RETURNFLAG;
    tpch_text_t       L_LINESTATUS;
    tpch_data_t       L_SHIPDATE;
    tpch_data_t       L_COMMITDATE;
    tpch_data_t       L_RECEIPTDATE;
    tpch_text_t       L_SHIPINSTRUCT;
    tpch_text_t       L_SHIPMODE;
    tpch_text_t       L_COMMENT;
} tpch_lineitem_tuple_t;

typedef struct
{
    char             *L_ORDERKEY;
    char             *L_PARTKEY;
    char             *L_SUPPKEY;
    char             *L_LINENUMBER;
    char             *L_QUANTITY;
    char             *L_EXTENDEDPRICE;
    char             *L_DISCOUNT;
    char             *L_TAX;
    char             *L_RETURNFLAG;
    char             *L_LINESTATUS;
    char             *L_SHIPDATE;
    char             *L_COMMITDATE;
    char             *L_RECEIPTDATE;
    char             *L_SHIPINSTRUCT;
    char             *L_SHIPMODE;
    char             *L_COMMENT;
} tpch_lineitem_tuple_str_t;