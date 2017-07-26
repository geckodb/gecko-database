
#include <mondrian.h>
#include <field_type.h>

//----------------------------------------------------------------------------------------------------------------------

static program_t *prog_print_text(const char *text);
static program_t *prog_create_table_okay();
static program_t *prog_create_table_userabort();
static program_t *prog_create_table_autoabort();


//----------------------------------------------------------------------------------------------------------------------

static void run_print_test();
static void run_create_tables();

//----------------------------------------------------------------------------------------------------------------------

int main(void)
{
    run_print_test();
    run_create_tables();

    return 0;
}

//----------------------------------------------------------------------------------------------------------------------

static program_t *prog_print_text(const char *text)
{
    program_t *program;

    BEGIN             (program)
        mvm_Print     (text)
        mvm_Commit    ()
    END               ()

    return program;
}


static program_t *prog_create_table_okay()
{
    program_t *program;

    BEGIN                    (program)
        mvm_CreateTable      ("MyTable")
        mvm_AddColumn        ("A1", FT_BOOL,    1, 0);
        mvm_AddColumn        ("A2", FT_CHAR,   42, 0);
        mvm_AddColumn        ("A3", FT_FLOAT32, 1, 0);
        mvm_EndTable         ()
        mvm_Commit           ()
    END                      ()

    return program;
}

static program_t *prog_create_table_userabort()
{
    program_t *program;

    BEGIN                    (program)
        mvm_CreateTable      ("MyTable")
        mvm_AddColumn        ("A1", FT_BOOL,    1, 0);
        mvm_AddColumn        ("A2", FT_CHAR,   42, 0);
        mvm_AddColumn        ("A3", FT_FLOAT32, 1, 0);
        mvm_Abort            ()
        mvm_EndTable         ()
        mvm_Commit           ()
    END                      ()

    return program;
}

static program_t *prog_create_table_autoabort()
{
    program_t *program;

    BEGIN                    (program)
        mvm_CreateTable      ("MyTable")
        mvm_AddColumn        ("A1", FT_BOOL,    1, 0);
        mvm_AddColumn        ("A1", FT_CHAR,   42, 0);  /* attribute name is given twice, auto abort */
        mvm_AddColumn        ("A3", FT_FLOAT32, 1, 0);
        mvm_Abort            ()
        mvm_EndTable         ()
        mvm_Commit           ()
    END                      ()

    return program;
}

//----------------------------------------------------------------------------------------------------------------------

static void run_print_test()
{
    mondrian_t *db;

    program_t *prog1 = prog_print_text("Hello Word!\n");
    program_t *prog2 = prog_print_text("Hello VM!\n");

    program_print(stdout, prog1);

    mondrian_open(&db);
    mvm_handle_t h1, h2, h3;

    mondrian_exec(&h1, db, prog1, future_lazy);
    mondrian_exec(&h2, db, prog2, future_eager);
    mondrian_exec(&h3, db, prog1, future_eager);

    mondrian_waitfor(&h1);
    mondrian_waitfor(&h2);
    mondrian_waitfor(&h3);

    mondrian_close(db);

    program_free(prog1);
    program_free(prog2);
}

static void run_create_tables()
{
    mondrian_t *db;

    program_t *prog1 = prog_create_table_userabort();
    program_t *prog2 = prog_create_table_autoabort();
    program_t *prog3 = prog_create_table_okay();
    program_t *prog4 = prog_create_table_okay();    // should abort, since table exists

    program_print(stdout, prog1);
    program_print(stdout, prog2);
    program_print(stdout, prog3);
    program_print(stdout, prog4);

    mondrian_open(&db);

    mondrian_exec(NULL, db, prog1, future_sync);
    mondrian_exec(NULL, db, prog2, future_sync);
    mondrian_exec(NULL, db, prog3, future_sync);
    mondrian_exec(NULL, db, prog4, future_sync);

    //mondrian_waitfor(&h1);

    mondrian_close(db);

    program_free(prog1);
}