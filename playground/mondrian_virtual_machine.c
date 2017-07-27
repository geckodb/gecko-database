
#include <mondrian.h>
#include <field_type.h>

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

    BEGIN                    (program, "programs", "Marcus Pinnecke", "Shows installed programs")

        ADD(                 MVM_OC_PUSH,       ACCESS_GLOBAL)
        ADD(                 MVM_OC_PUSH,       MODE_SHARED)
        ADD(                 MVM_OC_ACQ_LOCK,   CONTAINER_PROGPOOL)
        ADD(                 MVM_OC_VMOVE,      VARIABLE_LOCAL_0)

        ADD(                 MVM_OC_PROGLIST,   0)
        ADD(                 MVM_OC_VMOVE,      VARIABLE_RCX)

//        ADD(                 MVM_OC_PUSH,       ACCESS_GLOBAL)XXX
        ADD(                 MVM_OC_TCREATE,    FORCE_NO_FORCE)


        ADD(                 MVM_OC_PROGINFO,   0)
        ADD(                 MVM_OC_POP,        0)
        ADD(                 MVM_OC_DEC,        VARIABLE_RCX)
        ADD(                 MVM_OC_RJMP_NZ,    -3)

        ADD(                 MVM_OC_VLOAD,      VARIABLE_LOCAL_0)

        ADD(                 MVM_OC_REL_LOCK,   0)
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
    mvm_handle_t h1, h2;

    program_print(stdout, prog1);

    mondrian_open(&db);
    mondrian_install(&prog_id, db, prog1);
    mondrian_exec(&h1, db, prog_id, future_lazy);
    mondrian_exec(&h2, db, prog_id, future_lazy);
    mondrian_waitfor(&h1);
    mondrian_waitfor(&h2);
    mondrian_close(db);
    program_free(prog1);
}
