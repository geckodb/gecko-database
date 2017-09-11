#include <stdio.h>
#include <grid.h>
#include <tuplet_field.h>
#include <tuple_field.h>
#include <tuple.h>

void write_data(tuple_field_t *field, uint64_t a_field, uint32_t b_field, uint16_t c_field, uint16_t d_field);

int main(void) {

    schema_t *schema = gs_schema_create("My Grid Table");
    gs_attr_create_uint64("A", schema); // attribute id 0
    gs_attr_create_uint32("B", schema); // attribute id 1
    gs_attr_create_uint16("C", schema); // attribute id 2
    gs_attr_create_uint16("D", schema); // attribute id 3
    grid_table_t *table = gs_grid_table_create(schema, 16);
    attr_id_t cover[] = { 0, 1, 2, 3};

    tuple_t tuple;
    tuple_field_t field;
    tuple_cursor_t resultset;

    u64 a = 1;
    u32 b = 2;
    u16 c = 3;
    u16 d = 4;

    //           +===============================+
    // tuple id  |   A   |   B   |   C   |   D   |
    //           +===============================+

    //  Add full grid with same schema orientation as table, row-store
    //           +===============================+
    //           |   A   |   B   |   C   |   D   |
    //           +===============================+
    //         0 |   ------------------------>   |
    //         1 |   ------------------------>   |
    //         2 |   ------------------------>   |
    //           +===============================+

    tuple_id_interval_t g01_tid_cover[] = {
            { .begin = 0, .end = 3 }
    };
    grid_id_t g01 = gs_grid_table_add_grid(table, &cover[0], 4, &g01_tid_cover[0], 1, FIT_HOST_NSM_VM);

    gs_grid_table_insert(&resultset, table, 3);
    while (gs_tuple_cursor_next(&tuple, &resultset)) {
        gs_tuple_field_open(&field, &tuple);
        gs_tuple_field_write(&field, &a);
        gs_tuple_field_write(&field, &b);
        gs_tuple_field_write(&field, &c);
        gs_tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    gs_tuple_cursor_free(&resultset);
    gs_grid_print(stdout, table, g01, 0, UINT64_MAX);




    //  Add full grid with same schema orientation as table, column-store
    //           +===============================+
    //           |   A   |   B   |   C   |   D   |
    //           +===============================+
    //         3 |   |       |       |       |   |
    //         4 |   |       |       |       |   |
    //         5 |   v       v       v       v   |
    //           +===============================+
    tuple_id_interval_t g02_tid_cover[] = {
            { .begin = 3, .end = 6 }
    };
    grid_id_t g02 = gs_grid_table_add_grid(table, &cover[0], 4, &g02_tid_cover[0], 1, FIT_HOST_DSM_VM);

    gs_grid_table_insert(&resultset, table, 3);
    while (gs_tuple_cursor_next(&tuple, &resultset)) {
        gs_tuple_field_open(&field, &tuple);
        gs_tuple_field_write(&field, &a);
        gs_tuple_field_write(&field, &b);
        gs_tuple_field_write(&field, &c);
        gs_tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    gs_tuple_cursor_free(&resultset);
    gs_grid_print(stdout, table, g02, 0, UINT64_MAX);



    //  Add full grid with same altered schema orientation, row-store
    //           +===============================+
    //           |   D   |   C   |   B   |   A   |
    //           +===============================+
    //         6 |   ------------------------>   |
    //         7 |   ------------------------>   |
    //         8 |   ------------------------>   |
    //           +===============================+

    cover[0] = 3; cover[1] = 2; cover[2] = 1; cover[3] = 0;

    tuple_id_interval_t g03_tid_cover[] = {
            { .begin = 6, .end = 9 }
    };
    grid_id_t g03 = gs_grid_table_add_grid(table, &cover[0], 4, &g03_tid_cover[0], 1, FIT_HOST_NSM_VM);

    gs_grid_table_insert(&resultset, table, 3);
    while (gs_tuple_cursor_next(&tuple, &resultset)) {
        gs_tuple_field_open(&field, &tuple);
        gs_tuple_field_write(&field, &a);
        gs_tuple_field_write(&field, &b);
        gs_tuple_field_write(&field, &c);
        gs_tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    gs_tuple_cursor_free(&resultset);
    gs_grid_print(stdout, table, g03, 0, UINT64_MAX);
    gs_grid_table_grid_list_print(stdout, table, 0, UINT64_MAX);
    gs_grid_table_structure_print(stdout, table, 0, UINT64_MAX);

    /*cover[0] = 3; cover[2] = 2; cover[2] = 1; cover[3] = 0;
    interval_t g03_tid_cover[] = {
            { .begin = 6, .end = 9 }
    };
    grid_id_t g03 = gs_grid_table_add_grid(table, &cover[0], 4, &g03_tid_cover[0], 1, FIT_HOST_NSM_VM);

    tuple = gs_grid_table_insert(table, 3);
    tuplet_field = gs_tuple_field_open(tuple);
    write_data(tuplet_field, 24, 25, 26, 27);
    write_data(tuplet_field, 28, 29, 30, 31);
    write_data(tuplet_field, 32, 33, 34, 35);


    //  Add full grid with same altered schema orientation, column-store
    //           +===============================+
    //           |   D   |   C   |   B   |   A   |
    //           +===============================+
    //         9 |   |       |       |       |   |
    //        10 |   |       |       |       |   |
    //        11 |   v       v       v       v   |
    //           +===============================+
    interval_t g04_tid_cover[] = {
            { .begin = 9, .end = 12 }
    };
    grid_id_t g04 = gs_grid_table_add_grid(table, &cover[0], 4, &g04_tid_cover[0], 1, FIT_HOST_DSM_VM);

    tuple = gs_grid_table_insert(table, 3);
    tuplet_field = gs_tuple_field_open(tuple);
    write_data(tuplet_field, 36, 37, 38, 39);
    write_data(tuplet_field, 40, 41, 42, 43);
    write_data(tuplet_field, 44, 45, 46, 47);

    //  Add partial grids with same original schema orientation, column-store + row-store
    //           +===============|===============+
    //           |   A   |   B   |   C   |   D   |
    //           +===============|===============+
    //        11 |   |       |   |   -------->   |
    //        12 |   |       |   |   -------->   |
    //        13 |   v       v   |   -------->   |
    //           +===============|===============+
    cover[0] = 0; cover[2] = 1; cover[2] = 2; cover[3] = 3;
    interval_t g0506_tid_cover[] = {
            { .begin = 11, .end = 14 }
    };
    grid_id_t g05 = gs_grid_table_add_grid(table, &cover[0],     2, &g0506_tid_cover[0], 1, FIT_HOST_DSM_VM);
    grid_id_t g06 = gs_grid_table_add_grid(table, &cover[0] + 2, 2, &g0506_tid_cover[0], 1, FIT_HOST_NSM_VM);

    tuple = gs_grid_table_insert(table, 3);
    tuplet_field = gs_tuple_field_open(tuple);
    write_data(tuplet_field, 48, 49, 50, 51);
    write_data(tuplet_field, 52, 53, 54, 55);
    write_data(tuplet_field, 56, 57, 58, 59);

    //  Add partial grids with one having same original schema orientation other altered, column-store + row-store
    //           +===============|===============+
    //           |   A   |   B   |   D   |   C   |
    //           +===============|===============+
    //        14 |   |       |   |   -------->   |
    //        15 |   |       |   |   -------->   |
    //        16 |   v       v   |   -------->   |
    //           +===============|===============+
    cover[0] = 0; cover[2] = 1; cover[2] = 3; cover[3] = 2;
    interval_t g0708_tid_cover[] = {
            { .begin = 14, .end = 17 }
    };
    grid_id_t g07 = gs_grid_table_add_grid(table, &cover[0],     2, &g0708_tid_cover[0], 1, FIT_HOST_DSM_VM);
    grid_id_t g08 = gs_grid_table_add_grid(table, &cover[0] + 2, 2, &g0708_tid_cover[0], 1, FIT_HOST_NSM_VM);

    tuple = gs_grid_table_insert(table, 3);
    tuplet_field = gs_tuple_field_open(tuple);
    write_data(tuplet_field, 60, 61, 62, 63);
    write_data(tuplet_field, 64, 65, 66, 67);
    write_data(tuplet_field, 68, 69, 70, 71);

    //  Add 2 partial interleaved grids with original schema orientation, column-store + row-store
    //           +=======|===============|=======+
    //           |   A   |   B   |   C   |   D   |
    //           +=======|===============|=======+
    //        17 |  -->  |   |       |   |  -->  |
    //        18 |  -->  |   |       |   |  -->  |
    //        19 |  -->  |   v       v   |  -->  |
    //           +=======|=======================+
    cover[0] = 0; cover[1] = 3;
    interval_t g0910_tid_cover[] = {
            { .begin = 17, .end = 19 }
    };
    grid_id_t g09 = gs_grid_table_add_grid(table, &cover[0], 2, &g0910_tid_cover[0], 1, FIT_HOST_NSM_VM);
    cover[0] = 1; cover[1] = 2;
    grid_id_t g10 = gs_grid_table_add_grid(table, &cover[0], 2, &g0910_tid_cover[0], 1, FIT_HOST_DSM_VM);

    tuple = gs_grid_table_insert(table, 3);
    tuplet_field = gs_tuple_field_open(tuple);
    write_data(tuplet_field, 72, 73, 74, 75);
    write_data(tuplet_field, 76, 77, 78, 79);
    write_data(tuplet_field, 80, 81, 82, 83);

    //  Add 2 partial interleaved grids with one having alternate schema orientation, column-store + row-store
    //           +=======|===============|=======+
    //           |   D   |   B   |   C   |   A   |
    //           +=======|===============|=======+
    //        20 |  -->  |   |       |   |  -->  |
    //        21 |  -->  |   |       |   |  -->  |
    //        22 |  -->  |   v       v   |  -->  |
    //           +=======|=======================+
    cover[0] = 3; cover[1] = 0;
    interval_t g1112_tid_cover[] = {
            { .begin = 20, .end = 23 }
    };
    grid_id_t g11 = gs_grid_table_add_grid(table, &cover[0], 2, &g1112_tid_cover[0], 1, FIT_HOST_NSM_VM);
    cover[0] = 1; cover[1] = 2;
    grid_id_t g12 = gs_grid_table_add_grid(table, &cover[0], 2, &g1112_tid_cover[0], 1, FIT_HOST_DSM_VM);

    tuple = gs_grid_table_insert(table, 3);
    tuplet_field = gs_tuple_field_open(tuple);
    write_data(tuplet_field, 84, 85, 86, 87);
    write_data(tuplet_field, 88, 89, 90, 91);
    write_data(tuplet_field, 92, 93, 94, 95);

    //  Add complex interleaved grids with having alternate schema orientation, column-store + row-store
    //           +=======|=======================+
    //           |   D   |   B   |   C   |   A   |
    //           +=======|=======================+
    //        23 |   |   |   ----------------->  |
    //           |       |===============|=======+
    //           |       |   A   |   C   |   B   |
    //           |       |===============|=======+
    //        24 |   |   |   --------->  |   |   |
    //        25 |   |   |   --------->  |   |   |
    //        26 |   |   |   --------->  |   v   |
    //           |       |=======================+
    //           |       |   B   |   C   |   A   |
    //           |       |=======================+
    //        27 |   |   |   ----------------->  |
    //        28 |   v   |   ----------------->  |
    //           +=======|=======================+
    cover[0] = 3;
    interval_t g13_tid_cover[] = {
            { .begin = 23, .end = 29 }
    };
    grid_id_t g13 = gs_grid_table_add_grid(table, &cover[0], 1, &g13_tid_cover[0], 1, FIT_HOST_DSM_VM);

    cover[0] = 1; cover[1] = 2; cover[2] = 0;
    interval_t g14_tid_cover[] = {
            { .begin = 23, .end = 24 },
            { .begin = 27, .end = 29 },
    };
    grid_id_t g14 = gs_grid_table_add_grid(table, &cover[0], 3, &g14_tid_cover[0], 1, FIT_HOST_NSM_VM);

    cover[0] = 0; cover[1] = 2;
    interval_t g15_tid_cover[] = {
            { .begin = 24, .end = 27 },
    };
    grid_id_t g15 = gs_grid_table_add_grid(table, &cover[0], 2, &g15_tid_cover[0], 1, FIT_HOST_NSM_VM);

    cover[0] = 1;
    interval_t g16_tid_cover[] = {
            { .begin = 24, .end = 27 },
    };
    grid_id_t g16 = gs_grid_table_add_grid(table, &cover[0], 1, &g16_tid_cover[0], 1, FIT_HOST_DSM_VM);

    tuple = gs_grid_table_insert(table, 3);
    tuplet_field = gs_tuple_field_open(tuple);
    write_data(tuplet_field, 96, 97, 98, 99);
    write_data(tuplet_field, 100, 101, 102, 103);
    write_data(tuplet_field, 104, 105, 106, 107);
    write_data(tuplet_field, 108, 109, 110, 111);
    write_data(tuplet_field, 112, 113, 114, 115);
    write_data(tuplet_field, 116, 117, 118, 119);

    bool valid = gs_grid_table_is_valide(table);
    assert (valid);

    gs_grid_table_grid_print(stdout, table, g01, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g02, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g03, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g04, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g05, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g06, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g07, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g08, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g09, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g10, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g11, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g12, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g13, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g14, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g15, 0, UINT64_MAX);
    gs_grid_table_grid_print(stdout, table, g16, 0, UINT64_MAX);

    gs_grid_table_print(stdout, table, 0, UINT64_MAX);

*/

    gs_grid_table_print(stdout, table, 0, UINT64_MAX);
    gs_grid_table_free(table);
    free (table);
    gs_schema_free(schema);

    return 0;
}

void write_data(tuple_field_t *field, uint64_t a_field, uint32_t b_field, uint16_t c_field, uint16_t d_field)
{
    gs_tuple_field_write(field, &a_field);
    gs_tuple_field_write(field, &b_field);
    gs_tuple_field_write(field, &c_field);
    gs_tuple_field_write(field, &d_field);
}