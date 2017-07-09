#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <containers/vector.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <tableimg.h>

#define MAX_LINE_LEN                 2048
#define ALL_LINES_VECTOR_INIT_CAP   10000

/*
 * (TPCH-Spec) Identifier means that the column must be able to hold any key value generated for that column and be
 * able to support at least 2,147,483,647 unique values.
 */
typedef uint32_t          tpch_identifier_t;

/*
 * (TPCH-Spec) Integer means that the column must be able to exactly represent integer values (i.e., values in
 * increments of 1) in the range of at least -2,147,483,646 to 2,147,483,647.
 */
typedef int32_t           tpch_integer_t;

/*
 * (TPCH-Spec) Decimal means that the column must be able to represent values in the range -9,999,999,999.99 to
 * +9,999,999,999.99 in increments of 0.01; the values can be either represented exactly or interpreted to be in
 * this range.
 */
typedef int64_t           tpch_decimal_t;     /* Note: value is multipled with 100, e.g., 10.23 is stored as 1023 */

/*
 * (TPCH-Spec) Big Decimal is of the Decimal datatype as defined above, with the additional property that it must be
 * large enough to represent the aggregated values stored in temporary tables created within query variants;
 */
typedef double            tpch_big_decimal_t;

/*
 * (TPCH-Spec) Fixed text, size N means that the column must be able to hold any string of characters of a fixed
 * length of N.
 * (TPCH-Spec) Variable text, size N means that the column must be able to hold any string of characters of a variable
 * length with a maximum length of N. Columns defined as "variable text, size N" may optionally be implemented as
 * "fixed text, size N".
 */

#define                   TPCH_TEXT_N_11        11 + 1
#define                   TPCH_TEXT_N_10        10 + 1
#define                   TPCH_TEXT_N_15        15 + 1
#define                   TPCH_TEXT_N_23        23 + 1
#define                   TPCH_TEXT_N_25        25 + 1
#define                   TPCH_TEXT_N_40        40 + 1
#define                   TPCH_TEXT_N_44        44 + 1
#define                   TPCH_TEXT_N_55        55 + 1
#define                   TPCH_TEXT_N_79        79 + 1
#define                   TPCH_TEXT_N_101      101 + 1
#define                   TPCH_TEXT_N_117      117 + 1
#define                   TPCH_TEXT_N_152      152 + 1
#define                   TPCH_TEXT_N_199      199 + 1

typedef char             *tpch_text_t;

/*
 * (TPCH-Spec) Date is a value whose external representation can be expressed as YYYY-MM-DD, where all characters
 * are numeric. A date must be able to express any day within at least 14 consecutive years. There is no requirement
 * specific to the internal representation of a date.
 */
typedef uint64_t          tpch_data_t;        /* Note: string date is converted to unix timestamp */

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
    tpch_identifier_t PS_PARTKEY;
    tpch_identifier_t PS_SUPPKEY;
    tpch_integer_t    PS_AVAILQTY;
    tpch_decimal_t    PS_SUPPLYCOST;
    tpch_text_t       PS_COMMENT;
} tpch_partsupp_tuple_t;

typedef struct
{
    tpch_identifier_t C_CUSTKEY;
    tpch_text_t       C_NAME;
    tpch_text_t       C_ADDRESS;
    tpch_identifier_t C_NATIONKEY;
    tpch_text_t       C_PHONE;
    tpch_decimal_t    C_ACCTBAL;
    tpch_text_t       C_MKTSEGMENT;
    tpch_text_t       C_COMMENT;
} tpch_customer_tuple_t;

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
    tpch_identifier_t N_NATIONKEY;
    tpch_text_t       N_NAME;
    tpch_identifier_t N_REGIONKEY;
    tpch_text_t       N_COMMENT;
} tpch_nation_tuple_t;

typedef struct
{
    tpch_identifier_t R_REGIONKEY;
    tpch_text_t       R_NAME;
    tpch_text_t       R_COMMENT;
} tpch_region_tuple_t;


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

typedef struct
{
    char             *PS_PARTKEY;
    char             *PS_SUPPKEY;
    char             *PS_AVAILQTY;
    char             *PS_SUPPLYCOST;
    char             *PS_COMMENT;
} tpch_partsupp_tuple_str_t;

typedef struct
{
    char             *C_CUSTKEY;
    char             *C_NAME;
    char             *C_ADDRESS;
    char             *C_NATIONKEY;
    char             *C_PHONE;
    char             *C_ACCTBAL;
    char             *C_MKTSEGMENT;
    char             *C_COMMENT;
} tpch_customer_tuple_str_t;

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

typedef struct
{
    char             *N_NATIONKEY;
    char             *N_NAME;
    char             *N_REGIONKEY;
    char             *N_COMMENT;
} tpch_nation_tuple_str_t;

typedef struct
{
    char             *R_REGIONKEY;
    char             *R_NAME;
    char             *R_COMMENT;
} tpch_region_tuple_str_t;

uint64_t to_uint64(const char* str)
{
    uint64_t fact   = 1;
    uint64_t result = 0;
    int i = strlen(str) - 1;

    while (i--) {
        const char c = str[i];
        if (c == '-') {
            result *= -1;
        }
        if ((c == '0')|| (c == '1')|| (c == '2')|| (c == '3')|| (c == '4')|| (c == '5')||
            (c == '6')|| (c == '7')|| (c == '8')|| (c == '9')) {
            result = result + ((int)c - 48) * fact;
            fact *= 10;
        }
    }
    return result;
}

void identifer(tpch_identifier_t *dst, const char *src)
{
    *dst = (tpch_identifier_t) to_uint64(src);
}

void decimal(tpch_decimal_t *dst, const char *src)
{
    *dst = (tpch_decimal_t) to_uint64(src);
}

void text(tpch_text_t *dst, const char *src, size_t n)
{
    *dst = malloc(n);
    strcpy(*dst, src);
}

bool parse_customers(void *capture, void *begin, void *end)
{
    vector_t *result = (vector_t *) capture;
    for (const char **it = begin; it < (const char **) end; it ++) {
        const char *tuple_str = *it;
        tpch_customer_tuple_str_t tuple;
        char *running        = strdup(tuple_str);
        tuple.C_CUSTKEY      = strdup(strsep (&running, "|"));
        tuple.C_NAME         = strdup(strsep (&running, "|"));
        tuple.C_ADDRESS      = strdup(strsep (&running, "|"));
        tuple.C_NATIONKEY    = strdup(strsep (&running, "|"));
        tuple.C_PHONE        = strdup(strsep (&running, "|"));
        tuple.C_ACCTBAL      = strdup(strsep (&running, "|"));
        tuple.C_MKTSEGMENT   = strdup(strsep (&running, "|"));
        tuple.C_COMMENT      = strdup(strsep (&running, "|"));

        assert (strlen(tuple.C_NAME)         < TPCH_TEXT_N_25);
        assert (strlen(tuple.C_ADDRESS)      < TPCH_TEXT_N_40);
        assert (strlen(tuple.C_PHONE)        < TPCH_TEXT_N_15);
        assert (strlen(tuple.C_MKTSEGMENT)   < TPCH_TEXT_N_10);
        assert (strlen(tuple.C_COMMENT)     < TPCH_TEXT_N_117);

        vector_add(result, 1, &tuple);
    }
    return true;
}

bool free_customer_tuple_str_t(void *capture, void *begin, void *end) {
    for (tpch_customer_tuple_str_t *it = begin; it < (tpch_customer_tuple_str_t*) end; it++) {
        free (it->C_CUSTKEY);
        free (it->C_NAME);
        free (it->C_ADDRESS);
        free (it->C_NATIONKEY);
        free (it->C_PHONE);
        free (it->C_ACCTBAL);
        free (it->C_MKTSEGMENT);
        free (it->C_COMMENT);
    }
    return true;
}

bool convert_customer_tuple(void *capture, void *begin, void *end)
{
    vector_t *result = (vector_t *) capture;
    for (tpch_customer_tuple_str_t *it = begin; it < (tpch_customer_tuple_str_t *) end; it ++) {
        tpch_customer_tuple_t tuple;

        identifer(&tuple.C_CUSTKEY, it->C_CUSTKEY);
        identifer(&tuple.C_NATIONKEY, it->C_NATIONKEY);
        decimal(&tuple.C_ACCTBAL, it->C_ACCTBAL);
        text(&tuple.C_NAME, it->C_NAME,             TPCH_TEXT_N_25);
        text(&tuple.C_ADDRESS, it->C_ADDRESS,       TPCH_TEXT_N_40);
        text(&tuple.C_PHONE, it->C_PHONE,           TPCH_TEXT_N_15);
        text(&tuple.C_MKTSEGMENT, it->C_MKTSEGMENT, TPCH_TEXT_N_10);
        text(&tuple.C_COMMENT, it->C_COMMENT,       TPCH_TEXT_N_117);

        vector_add(result, 1, &tuple);
    }
    return true;
}

bool free_customer_tuple_t(void *capture, void *begin, void *end) {
    for (tpch_customer_tuple_t *it = begin; it < (tpch_customer_tuple_t*) end; it++) {
        free (it->C_NAME);
        free (it->C_ADDRESS);
        free (it->C_PHONE);
        free (it->C_MKTSEGMENT);
        free (it->C_COMMENT);
    }
    return true;
}

bool serialize_customer_table(void *capture, void *begin, void *end) {
//    serialization_config_t *config = (serialization_config_t *) capture;


    /*switch (config->format) {
        case SF_NSM:
            for (tpch_customer_tuple_t *it = begin; it < (tpch_customer_tuple_t*) end; it++) {
                fwrite(&header, sizeof(timg_header_t), 1, file_ptr);
            }
            fwrite(&header, sizeof(timg_header_t), 1, file_ptr);
            break;
        case SF_DSM:
            for (tpch_customer_tuple_t *it = begin; it < (tpch_customer_tuple_t*) end; it++)
                printf("%d\n", it->C_CUSTKEY);
            for (tpch_customer_tuple_t *it = begin; it < (tpch_customer_tuple_t*) end; it++)
                printf("%s\n", it->C_NAME);

            break;
        default:
            perror("Unknown serialization format requested.");
            abort();
    }*/
    return true;
}

void for_each_line(
    const char *file,

    serialization_config_t *config,

    size_t tuple_str_t_size,
    bool parse(void *capture, void *begin, void *end),
    bool free_parse_result(void *capture, void *begin, void *end),

    size_t tuple_t_size,
    bool convert(void *capture, void *begin, void *end),
    bool free_convert_result(void *capture, void *begin, void *end),

    bool serialize(void *capture, void *begin, void *end))
{
    vector_t *all_lines = vector_create(sizeof(char *), ALL_LINES_VECTOR_INIT_CAP);
    char *line = malloc(MAX_LINE_LEN);
    FILE *file_ptr = fopen(file, "r");

    while (fgets(line, MAX_LINE_LEN, file_ptr) != NULL) {
        char *line_cpy = strdup(line);
        vector_add(all_lines, 1, &line_cpy);
    }
    free (line);

    vector_t *parse_result = vector_create(tuple_str_t_size, ALL_LINES_VECTOR_INIT_CAP);
    vector_foreach(all_lines, parse_result, parse);
    vector_free__str(all_lines);

    vector_t *convert_result = vector_create(tuple_t_size, ALL_LINES_VECTOR_INIT_CAP);
    vector_foreach(parse_result, convert_result, convert);
    vector_free_ex(parse_result, NULL, free_parse_result);

    vector_foreach(convert_result, config, serialize);
    vector_free_ex(convert_result, NULL, free_convert_result);





}

int main() {

    printf("This is tpch convert. Copyright (c) 2017 Marcus Pinnecke\n"
           "This tools converts the textual database tables as generated by tpch-dbgen into table image files (*timg).\n"
           "A table image file is an easy to use serialization format that supports NSM and DSM physical layouts.\n\n"
           "For details on the binary format, take a look at the source of tpch convert (/utils/tpch-convert/laucher.c)");

    serialization_config_t config = {
        .dest_file = "bench/olap/tpch/database/customer.timg",
        .format    = SF_DSM
    };

    for_each_line("../sbin/tpch-dbgen/customer.tbl",
                  &config,
                  sizeof(tpch_customer_tuple_str_t),
                  parse_customers,
                  free_customer_tuple_str_t,
                  sizeof(tpch_customer_tuple_t),
                  convert_customer_tuple,
                  free_customer_tuple_t,
                  serialize_customer_table);
    // bench/olap/tpch/database


    const size_t NUM_TUPLES = 4;

    timg_schema_t schema = timg_schema_create();

    timg_attr_id_t attr_1 = timg_attribute_create_uint64 ("My Attribute",       schema);
    timg_attr_id_t attr_2 = timg_attribute_create_string("My Attribute 2", 42, schema);
    timg_attr_id_t attr_3 = timg_attribute_create_bool  ("My Attribute Bool",  schema);

    void *data = timg_table_create(schema, NUM_TUPLES);

    void *cont = NULL;
    uint64_t int_value = 10;
    char *str_value = "Hello World\n";
    bool bol_value = true;

    int_value = 1;                cont = timg_field_write_uint64(data, schema, attr_1, &int_value);
    str_value = "Hello\n";        cont = timg_field_write_string(cont, schema, attr_2, str_value);
    bol_value = true;             cont = timg_field_write_bool  (cont, schema, attr_3, &bol_value);

    int_value = 2;                cont = timg_field_write_uint64(data, schema, attr_1, &int_value);
    str_value = "World\n";        cont = timg_field_write_string(cont, schema, attr_2, str_value);
    bol_value = false;            cont = timg_field_write_bool  (cont, schema, attr_3, &bol_value);

    int_value = 3;                cont = timg_field_write_uint64(data, schema, attr_1, &int_value);
    str_value = "Hi\n";           cont = timg_field_write_string(cont, schema, attr_2, str_value);
    bol_value = true;             cont = timg_field_write_bool  (cont, schema, attr_3, &bol_value);

    int_value = 4;                cont = timg_field_write_uint64(data, schema, attr_1, &int_value);
    str_value = "There\n";        cont = timg_field_write_string(cont, schema, attr_2, str_value);
    bol_value = false;            cont = timg_field_write_bool  (cont, schema, attr_3, &bol_value);

    FILE *file_ptr = fopen("/Users/marcus/temp/demo.timg", "w");
    tableimg_fwrite(file_ptr,
                    TIMG_VER_1,
                    "My Demo Database",
                    "My Table Name",
                    "www.ich.de",
                    "scale factor 1",
                    schema,
                    cont,
                    NUM_TUPLES,
                    SF_NSM,
                    SF_NSM
    );

    return EXIT_SUCCESS;
}