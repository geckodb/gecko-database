#pragma once

#include <stdinc.h>

//----------------------------------------------------------------------------------------------------------------------
// F O R W A R D S
//----------------------------------------------------------------------------------------------------------------------

typedef struct mondrian_t mondrian_t;

//----------------------------------------------------------------------------------------------------------------------
// T Y P E S
//----------------------------------------------------------------------------------------------------------------------

typedef struct mondrian_vm_t mondrian_vm_t;

typedef struct program_t program_t;

typedef struct instruction_t
{
    u32 opcode;
    u64 op1, op2, op3, op4;
} instruction_t;

#define MONDRIAN_OPCODE_COMMIT               0x0000
#define MONDRIAN_OPCODE_ABORT                0x0001
#define MONDRIAN_OPCODE_PRINT                0x0002
#define MONDRIAN_OPCODE_ADD_COLUMN           0x0003
#define MONDRIAN_OPCODE_CREATE_TABLE         0x0004
#define MONDRIAN_OPCODE_END_TABLE            0x0005


static struct {
    u32 opcode;
    const char mnemonic[256];
} opcode_mnemonics[] = {
    { MONDRIAN_OPCODE_COMMIT,  "Commit"},
    { MONDRIAN_OPCODE_ABORT,  "Abort"},
    { MONDRIAN_OPCODE_PRINT, "Print"},
    { MONDRIAN_OPCODE_ADD_COLUMN, "AddColumn"},
    { MONDRIAN_OPCODE_CREATE_TABLE, "CreateTable" },
    { MONDRIAN_OPCODE_END_TABLE,   "EndTable" }
} ;

#define mvm_Commit()                                    \
{                                                       \
    instruction_t inst = {                              \
        .opcode    = MONDRIAN_OPCODE_COMMIT             \
    };                                                  \
    program_add(prog, &inst);                           \
}

#define mvm_Abort()                                     \
{                                                       \
    instruction_t inst = {                              \
        .opcode    = MONDRIAN_OPCODE_ABORT              \
    };                                                  \
    program_add(prog, &inst);                           \
}

#define mvm_Print(/*const char * */ str)                \
{                                                       \
    instruction_t inst = {                              \
        .opcode    = MONDRIAN_OPCODE_PRINT,             \
        .op1       = (u64) str                          \
    };                                                  \
    program_add(prog, &inst);                           \
}

#define mvm_CreateTable(/*const char * */ table_name)                                                                  \
{                                                                                                                      \
    instruction_t inst = {                                                                                             \
        .opcode    = MONDRIAN_OPCODE_CREATE_TABLE,                                                                     \
        .op1       = (u64) table_name                                                                                  \
    };                                                                                                                 \
    program_add(prog, &inst);                                                                                          \
}

#define mvm_EndTable()                                                                                                \
{                                                                                                                      \
    instruction_t inst = {                                                                                             \
        .opcode    = MONDRIAN_OPCODE_END_TABLE,                                                                       \
    };                                                                                                                 \
    program_add(prog, &inst);                                                                                          \
}

#define mvm_AddColumn(/*const char * */ name, /*unsigned*/ type, /*unsigned*/ rep, /*unsigned*/ flags)                 \
{                                                                                                                      \
    instruction_t inst = {                                                                                             \
        .opcode    = MONDRIAN_OPCODE_ADD_COLUMN,                                                                       \
        .op1       = type,                                                                                             \
        .op2       = (u64) name,                                                                                       \
        .op3       = rep,                                                                                              \
        .op4       = flags                                                                                             \
    };                                                                                                                 \
    program_add(prog, &inst);                                                                                          \
}

#define BEGIN(/* program_t * */ out)                                                                                   \
{                                                                                                                      \
    program_t *prog;                                                                                                   \
    program_new(&prog);                                                                                                \
    out = prog;

#define END()                                                                                                          \
}


int mondrian_vm_create(mondrian_vm_t **out, mondrian_t *db);

int mondrian_vm_run(mondrian_vm_t *vm, const program_t *program, int *return_value);

int mondrian_vm_reset(mondrian_vm_t *vm);

int mondrian_vm_free(mondrian_vm_t *vm);

int program_new(program_t **out);

int program_add(program_t *out, instruction_t *inst);

int program_print(FILE *out, const program_t *program);

int program_free(program_t *program);



