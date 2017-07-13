#pragma once

typedef struct
{
    tpch_identifier_t O_ORDERKEY;
    tpch_identifier_t O_CUSTKEY;
    tpch_text_t       O_ORDERSTATUS;
    tpch_decimal_t    O_TOTALPRICE;
    tpch_data_t       O_ORDERDATE;
    tpch_text_t       O_ORDERPRIORITY;
    tpch_text_t       O_CLERK;
    tpch_integer_t    O_SHIPPRIORITY;
    tpch_text_t       O_COMMENT;
} tpch_orders_tuple_t;

typedef struct
{
    char             *O_ORDERKEY;
    char             *O_CUSTKEY;
    char             *O_ORDERSTATUS;
    char             *O_TOTALPRICE;
    char             *O_ORDERDATE;
    char             *O_ORDERPRIORITY;
    char             *O_CLERK;
    char             *O_SHIPPRIORITY;
    char             *O_COMMENT;
} tpch_orders_tuple_str_t;