#pragma once

#include <stdinc.h>

#define MAX_LINE_LEN                 2048
#define ALL_LINES_VECTOR_INIT_CAP   10000


typedef struct {
    const char *dest_file;
    enum tuplet_format format;
} serialization_config_t;

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