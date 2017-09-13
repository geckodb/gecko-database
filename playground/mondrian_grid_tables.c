// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <grid.h>
#include <tuplet_field.h>
#include <tuple_field.h>
#include <tuple.h>

void write_data(tuple_field_t *field, uint64_t a_field, uint32_t b_field, uint16_t c_field, uint16_t d_field);

int main(void) {

    schema_t *schema = schema_new("My Grid Table");
    attr_create_uint64("A", schema); // attribute id 0
    attr_create_uint32("B", schema); // attribute id 1
    attr_create_uint16("C", schema); // attribute id 2
    attr_create_uint16("D", schema); // attribute id 3
    table_t *table = table_new(schema, 16);
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
    grid_id_t g01 = table_add(table, &cover[0], 4, &g01_tid_cover[0], 1, FIT_HOST_NSM_VM);

    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 0:\n");
    grid_print(stdout, table, g01, 0, UINT64_MAX);




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
    grid_id_t g02 = table_add(table, &cover[0], 4, &g02_tid_cover[0], 1, FIT_HOST_DSM_VM);

    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 1:\n");
    grid_print(stdout, table, g02, 0, UINT64_MAX);



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
    grid_id_t g03 = table_add(table, &cover[0], 4, &g03_tid_cover[0], 1, FIT_HOST_NSM_VM);

    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 2:\n");
    grid_print(stdout, table, g03, 0, UINT64_MAX);

    //  Add full grid with same altered schema orientation, column-store
    //           +===============================+
    //           |   D   |   C   |   B   |   A   |
    //           +===============================+
    //         9 |   |       |       |       |   |
    //        10 |   |       |       |       |   |
    //        11 |   v       v       v       v   |
    //           +===============================+
    cover[0] = 3; cover[1] = 2; cover[2] = 1; cover[3] = 0;

    tuple_id_interval_t g04_tid_cover[] = {
            { .begin = 9, .end = 12 }
    };
    grid_id_t g04 = table_add(table, &cover[0], 4, &g04_tid_cover[0], 1, FIT_HOST_DSM_VM);

    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 3:\n");
    grid_print(stdout, table, g04, 0, UINT64_MAX);

    //  Add partial grids with same original schema orientation, column-store + row-store
    //           +===============|===============+
    //           |   A   |   B   |   C   |   D   |
    //           +===============|===============+
    //        12 |   |       |   |   -------->   |
    //        13 |   |       |   |   -------->   |
    //        14 |   v       v   |   -------->   |
    //           +===============|===============+

    cover[0] = 0; cover[1] = 1; cover[2] = 2; cover[3] = 3;

    tuple_id_interval_t g0506_tid_cover[] = {
            { .begin = 12, .end = 15 }
    };

    grid_id_t g07 = table_add(table, &cover[0], 2, &g0506_tid_cover[0], 1, FIT_HOST_DSM_VM);
    grid_id_t g08 = table_add(table, &cover[0] + 2, 2, &g0506_tid_cover[0], 1, FIT_HOST_NSM_VM);


    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 4:\n");
    grid_print(stdout, table, g07, 0, UINT64_MAX);
    printf("grid 5:\n");
    grid_print(stdout, table, g08, 0, UINT64_MAX);

    //  Add partial grids with one having same original schema orientation other altered, column-store + row-store
    //           +===============|===============+
    //           |   A   |   B   |   D   |   C   |
    //           +===============|===============+
    //        15 |   |       |   |   -------->   |
    //        16 |   |       |   |   -------->   |
    //        17 |   v       v   |   -------->   |
    //           +===============|===============+

    cover[0] = 0; cover[1] = 1; cover[2] = 3; cover[3] = 2;

    tuple_id_interval_t g0910_tid_cover[] = {
            { .begin = 15, .end = 18 }
    };

    grid_id_t g09 = table_add(table, &cover[0], 2, &g0910_tid_cover[0], 1, FIT_HOST_DSM_VM);
    grid_id_t g10 = table_add(table, &cover[0] + 2, 2, &g0910_tid_cover[0], 1, FIT_HOST_NSM_VM);

    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 6:\n");
    grid_print(stdout, table, g09, 0, UINT64_MAX);
    printf("grid 7:\n");
    grid_print(stdout, table, g10, 0, UINT64_MAX);


  //  Add 2 partial interleaved grids with original schema orientation, column-store + row-store
  //           +=======|===============|=======+
  //           |   A   |   B   |   C   |   D   |
  //           +=======|===============|=======+
  //        18 |  -->  |   |       |   |  -->  |
  //        19 |  -->  |   |       |   |  -->  |
  //        20 |  -->  |   v       v   |  -->  |
  //           +=======|=======================+

    cover[0] = 0; cover[1] = 3; cover[2] = 1; cover[3] = 2;

    tuple_id_interval_t g1112_tid_cover[] = {
            { .begin = 18, .end = 21 }
    };

    grid_id_t g11 = table_add(table, &cover[0], 2, &g1112_tid_cover[0], 1, FIT_HOST_DSM_VM);
    grid_id_t g12 = table_add(table, &cover[0] + 2, 2, &g1112_tid_cover[0], 1, FIT_HOST_NSM_VM);

    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 8:\n");
    grid_print(stdout, table, g11, 0, UINT64_MAX);
    printf("grid 9:\n");
    grid_print(stdout, table, g12, 0, UINT64_MAX);


  //  Add 2 partial interleaved grids with one having alternate schema orientation, column-store + row-store
  //           +=======|===============|=======+
  //           |   D   |   B   |   C   |   A   |
  //           +=======|===============|=======+
  //        21 |  -->  |   |       |   |  -->  |
  //        22 |  -->  |   |       |   |  -->  |
  //        23 |  -->  |   v       v   |  -->  |
  //           +=======|=======================+

    cover[0] = 3; cover[1] = 0; cover[2] = 1; cover[3] = 2;

    tuple_id_interval_t g1314_tid_cover[] = {
            { .begin = 21, .end = 24 }
    };

    grid_id_t g13 = table_add(table, &cover[0], 2, &g1314_tid_cover[0], 1, FIT_HOST_DSM_VM);
    grid_id_t g14 = table_add(table, &cover[0] + 2, 2, &g1314_tid_cover[0], 1, FIT_HOST_NSM_VM);

    grid_insert(&resultset, table, 3);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 10:\n");
    grid_print(stdout, table, g13, 0, UINT64_MAX);
    printf("grid 11:\n");
    grid_print(stdout, table, g14, 0, UINT64_MAX);


  //  Add complex interleaved grid layout with having alternate schema orientation, column-store + row-store
  //           +=======|=======================+
  //           |   D   |   B   |   C   |   A   |
  //           +=======|=======================+
  //        24 |   |   |   ----------------->  |
  //           |       |===============|=======+
  //           |       |   A   |   C   |   B   |
  //           |       |===============|=======+
  //        25 |   |   |   --------->  |   |   |
  //        26 |   |   |   --------->  |   |   |
  //        27 |   |   |   --------->  |   v   |
  //           |       |=======================+
  //           |       |   B   |   C   |   A   |
  //           |       |=======================+
  //        28 |   |   |   ----------------->  |
  //        29 |   v   |   ----------------->  |
  //           +=======|=======================+

    cover[0] = 3; cover[1] = 1; cover[2] = 2; cover[3] = 0;

    tuple_id_interval_t tid_cover[] = {
            { .begin = 24, .end = 30 }
    };

    grid_id_t g15 = table_add(table, &cover[0], 1, &tid_cover[0], 1, FIT_HOST_DSM_VM);

    tuple_id_interval_t tid_cover2[] = {
            { .begin = 24, .end = 25 }, { .begin = 28, .end = 30 }
    };

    grid_id_t g16 = table_add(table, &cover[0] + 1, 3, &tid_cover2[0], 2, FIT_HOST_NSM_VM);

    cover[0] = 0; cover[1] = 2;
    tuple_id_interval_t tid_cover3[] = { { .begin = 25, .end = 28 }};
    grid_id_t g17 = table_add(table, &cover[0], 2, &tid_cover3[0], 1, FIT_HOST_NSM_VM);

    cover[0] = 1;
    tuple_id_interval_t tid_cover4[] = { { .begin = 25, .end = 28 }};
    grid_id_t g18 = table_add(table, &cover[0], 1, &tid_cover4[0], 1, FIT_HOST_DSM_VM);

    grid_insert(&resultset, table, 6);
    while (tuple_cursor_next(&tuple, &resultset)) {
        tuple_field_open(&field, &tuple);
        tuple_field_write(&field, &a);
        tuple_field_write(&field, &b);
        tuple_field_write(&field, &c);
        tuple_field_write(&field, &d);
        a += 4;
        b += 4;
        c += 4;
        d += 4;
    }

    tuple_cursor_dispose(&resultset);
    printf("grid 12:\n");
    grid_print(stdout, table, g15, 0, UINT64_MAX);
    printf("grid 13:\n");
    grid_print(stdout, table, g16, 0, UINT64_MAX);
    printf("grid 14:\n");
    grid_print(stdout, table, g17, 0, UINT64_MAX);
    printf("grid 15:\n");
    grid_print(stdout, table, g18, 0, UINT64_MAX);

    printf("grid table schema:\n");
    schema_print(stdout, table->schema);
    printf("grid table grid list:\n");
    table_grid_list_print(stdout, table, 0, UINT64_MAX);
    printf("grid table structure:\n");
    table_structure_print(stdout, table, 0, UINT64_MAX);
    printf("grid table:\n");
    table_print(stdout, table, 0, UINT64_MAX);
    printf("grid table hindex:\n");
    hindex_print(stdout, table->tuple_cover);
    printf("grid table vindex:\n");
    vindex_print(stdout, table->schema_cover);

    table_delete(table);
    free (table);
    schema_delete(schema);

    return 0;
}

void write_data(tuple_field_t *field, uint64_t a_field, uint32_t b_field, uint16_t c_field, uint16_t d_field)
{
    tuple_field_write(field, &a_field);
    tuple_field_write(field, &b_field);
    tuple_field_write(field, &c_field);
    tuple_field_write(field, &d_field);
}