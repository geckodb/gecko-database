#pragma once

#include <schema.h>
#include <frag.h>
#include <field.h>

#include "../types.h"
#include "../common.h"

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
    char             *C_CUSTKEY;
    char             *C_NAME;
    char             *C_ADDRESS;
    char             *C_NATIONKEY;
    char             *C_PHONE;
    char             *C_ACCTBAL;
    char             *C_MKTSEGMENT;
    char             *C_COMMENT;
} tpch_customer_tuple_str_t;

bool parse_customers(void *capture, void *begin, void *end)
{
    vector_t *result = (vector_t *) capture;
    for (const char **it = begin; it < (const char **) end; it ++) {
        const char *tuple_str = *it;
        tpch_customer_tuple_str_t tuple;
        char *running        = strdup(tuple_str);
        char *tofree         = running;
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
        free (tofree);
    }
    return true;
}

static inline bool free_customer_tuple_str_t(void *capture, void *begin, void *end) {
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

static inline bool convert_customer_tuple(void *capture, void *begin, void *end)
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

static inline bool free_customer_tuple_t(void *capture, void *begin, void *end) {
    for (tpch_customer_tuple_t *it = begin; it < (tpch_customer_tuple_t*) end; it++) {
        free (it->C_NAME);
        free (it->C_ADDRESS);
        free (it->C_PHONE);
        free (it->C_MKTSEGMENT);
        free (it->C_COMMENT);
    }
    return true;
}

static inline bool serialize_customer_table(void *capture, void *begin, void *end) {
    serialization_config_t *config = (serialization_config_t *) capture;

    schema_t *schema = gs_schema_create();

    assert (sizeof(tpch_identifier_t) == sizeof(UINT32));
    assert (sizeof(tpch_text_t) == sizeof(CHAR * ));
    assert (sizeof(tpch_decimal_t) == sizeof(INT64));

    gs_attr_create_uint32("c_custkey", schema);
    gs_attr_create_string("c_name", TPCH_TEXT_N_25, schema);
    gs_attr_create_string("c_address", TPCH_TEXT_N_40, schema);
    gs_attr_create_uint32("c_nationkey", schema);
    gs_attr_create_string("c_phone", TPCH_TEXT_N_15, schema);
    gs_attr_create_uint64("c_acctbal", schema);
    gs_attr_create_string("c_mktsegment", TPCH_TEXT_N_10, schema);
    gs_attr_create_string("c_comment", TPCH_TEXT_N_117, schema);

    size_t num_tuplets = ((tpch_customer_tuple_t *)end - (tpch_customer_tuple_t *)begin);
    frag_t *fragment = gs_fragment_alloc(schema, num_tuplets, config->format);
    tuplet_t *tuplet = gs_fragment_insert(fragment, num_tuplets);
    field_t *field = gs_field_open(tuplet);

    size_t j = 0;
    for (tpch_customer_tuple_t *it = begin; it < (tpch_customer_tuple_t *) end; it++) {

        printf("%zu: %d | %s | %s | %d | %s ...\n", j++, it->C_CUSTKEY, it->C_NAME, it->C_ADDRESS, it->C_NATIONKEY, it->C_PHONE);

        gs_field_write(field, &it->C_CUSTKEY);
        gs_field_write(field, it->C_NAME);
        gs_field_write(field, it->C_ADDRESS);
        gs_field_write(field, &it->C_NATIONKEY);
        gs_field_write(field, it->C_PHONE);
        gs_field_write(field, &it->C_ACCTBAL);
        gs_field_write(field, it->C_MKTSEGMENT);
        gs_field_write(field, it->C_COMMENT);
    }

    printf("DONE\n");

    printf("\n");
    gs_frag_print(stdout, fragment, 0, 100);
    printf("\n");

    gs_schema_free(schema);
    gs_fragment_free(fragment);

    return true;
}