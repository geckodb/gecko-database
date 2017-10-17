// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
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

#include <mondrian.h>
#include <gs_field_type.h>

//----------------------------------------------------------------------------------------------------------------------

static program_t *prog_test();


//----------------------------------------------------------------------------------------------------------------------

static void run_test();

//----------------------------------------------------------------------------------------------------------------------

int main(void)
{
    run_test();

    return 0;
}

//----------------------------------------------------------------------------------------------------------------------

static program_t *prog_test()
{
    program_t *program;

    BEGIN                    (program, "hello_world", "Marcus Pinnecke", "Shows installed programs")

        /* acquire exclusive lock to program pool (since shared is not implemented, yet) */
        ADD(                 MVM_OC_PUSH,       ACCESS_GLOBAL)
        ADD(                 MVM_OC_PUSH,       MODE_EXCLUSIVE)
        ADD(                 MVM_OC_ACQ_LOCK,   CONTAINER_PROGPOOL)
        ADD(                 MVM_OC_VMOVE,      VARIABLE_LOCAL_0)

        /* get number of installed programs and store this number in counter variable */
        ADD(                 MVM_OC_PROGLIST,   0)
        ADD(                 MVM_OC_VMOVE,      VARIABLE_RCX)

        /* create table in local space in order to display program information */
        ADD(                 MVM_OC_TEMP_NAME,  0)
        ADD(                 MVM_OC_PUSH,       FRAGMENT_SCOPE_LOCAL)
        ADD(                 MVM_OC_FCREATE,    FORCE_NO_FORCE)

        ADD(                 MVM_OC_PUSH,       FLAG_PRIMARY)
        ADD(                 MVM_OC_PUSH,       1)
        ADD(                 MVM_OC_PUSH,       FT_UINT64)
        ADD(                 MVM_OC_PUSH,       (u64) &STRING_BUILTIN_PROG_ID[0])
        ADD(                 MVM_OC_ADDATTR,     0)
        ADD(                 MVM_OC_POP,         0)

        ADD(                 MVM_OC_PUSH,       FLAG_REGULAR)
        ADD(                 MVM_OC_PUSH,       PROGRAM_MAX_NAME_LENGTH)
        ADD(                 MVM_OC_PUSH,       FT_CHAR)
        ADD(                 MVM_OC_PUSH,       (u64) &STRING_BUILTIN_NAME[0])
        ADD(                 MVM_OC_ADDATTR,     0)
        ADD(                 MVM_OC_POP,         0)

        ADD(                 MVM_OC_PUSH,       FLAG_REGULAR)
        ADD(                 MVM_OC_PUSH,       PROGRAM_MAX_AUTHOR_LENGTH)
        ADD(                 MVM_OC_PUSH,       FT_CHAR)
        ADD(                 MVM_OC_PUSH,       (u64) &STRING_BUILTIN_AUTHOR[0])
        ADD(                 MVM_OC_ADDATTR,     0)
        ADD(                 MVM_OC_POP,         0)

        ADD(                 MVM_OC_PUSH,       FLAG_REGULAR)
        ADD(                 MVM_OC_PUSH,       PROGRAM_MAX_COMMENT_LENGTH)
        ADD(                 MVM_OC_PUSH,       FT_CHAR)
        ADD(                 MVM_OC_PUSH,       (u64) &STRING_BUILTIN_COMMENT[0])
        ADD(                 MVM_OC_ADDATTR,     0)
        ADD(                 MVM_OC_POP,         0)

        ADD(                 MVM_OC_PUSH,       FLAG_REGULAR)
        ADD(                 MVM_OC_PUSH,       1)
        ADD(                 MVM_OC_PUSH,       FT_UINT64)
        ADD(                 MVM_OC_PUSH,       (u64) &STRING_BUILTIN_SIZE[0])
        ADD(                 MVM_OC_ADDATTR,     0)
        ADD(                 MVM_OC_POP,         0)

        ADD(                 MVM_OC_VLOAD,      VARIABLE_RCX)
        ADD(                 MVM_OC_PUSH,       FRAGMENT_HOST_PLAIN_ROW_STORE)
        ADD(                 MVM_OC_PUSH,       FRAGMENT_SCOPE_LOCAL)
        ADD(                 MVM_OC_FINSTALL,   0)
        ADD(                 MVM_OC_VSTORE,     VARIABLE_LOCAL_1)

        /* this_query as many new tuplets into the fragments as programs are installed */
        ADD(                 MVM_OC_VLOAD,      VARIABLE_RCX)
        ADD(                 MVM_OC_FINSERT,    0)
        ADD(                 MVM_OC_OFIELD,     0)

        /* get program information, push these information into the local table, and loop for all programs */
        ADD(                 MVM_OC_PROGINFO,   0)
        ADD(                 MVM_OC_WFIELD,     0)
        ADD(                 MVM_OC_WFIELD,     0)
        ADD(                 MVM_OC_WFIELD,     0)
        ADD(                 MVM_OC_WFIELD,     0)
        ADD(                 MVM_OC_WFIELD,     0)
        ADD(                 MVM_OC_DEC,        VARIABLE_RCX)
        ADD(                 MVM_OC_RJMP_NZ,    -7)

        /* print local fragment to stdout */
        ADD(                 MVM_OC_VLOAD,      VARIABLE_LOCAL_1)
        ADD(                 MVM_OC_PRINT,      0)

        /* release exclusive lock on the program pool */
        ADD(                 MVM_OC_VLOAD,      VARIABLE_LOCAL_0)
        ADD(                 MVM_OC_REL_LOCK,   0)

        /* successfully exit this program */
        ADD(                 MVM_OC_COMMIT,     0)
    END                      ()

    return program;
}

//----------------------------------------------------------------------------------------------------------------------

static void run_test()
{
    mondrian_t *db;

    program_t *prog1 = prog_test();
    prog_id_t prog_id;
    mvm_handle_t h1;//, h2;

    program_print(stdout, prog1);

    mondrian_open(&db);
    mondrian_show_debug_log(db, false);
    mondrian_install(&prog_id, db, prog1);
    mondrian_exec(&h1, db, prog_id, future_lazy);
    //mondrian_exec(&h2, db, prog_id, future_lazy);
    mondrian_waitfor(&h1);
    //mondrian_waitfor(&h2);
    mondrian_close(db);
    program_free(prog1);
}
