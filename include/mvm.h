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
    u8  opcode;
    u64 operand;
} instruction_t;

#define MVM_OC_ABORT                0x00
#define MVM_OC_ADDCOL               0x01
#define MVM_OC_BYNAME               0x02
#define MVM_OC_COMMIT               0x03
#define MVM_OC_DCLONE               0x04
#define MVM_OC_DOPEN                0x05
#define MVM_OC_RJMP_NZ              0x06
#define MVM_OC_FWRITE               0x07
#define MVM_OC_PUSH                 0x08
#define MVM_OC_ISSET                0x09
#define MVM_OC_NPOP                 0x0A
#define MVM_OC_POP                  0x0B
#define MVM_OC_SLEEP                0x0C
#define MVM_OC_SWAP                 0x0D
#define MVM_OC_TCREATE              0x0E
#define MVM_OC_TDROP                0x0F
#define MVM_OC_TDUP                 0x10
#define MVM_OC_TINFO                0x11
#define MVM_OC_TINSERT              0x12
#define MVM_OC_TINSTALL             0x13
#define MVM_OC_TLIST                0x14
#define MVM_OC_PROGCLEAN            0x15
#define MVM_OC_PROGINFO             0x16
#define MVM_OC_PROGLIST             0x17
#define MVM_OC_VLOAD                0x18
#define MVM_OC_VSTORE               0x19
#define MVM_OC_JLIST                0x1A
#define MVM_OC_JINFO                0x1B
#define MVM_OC_JCLEAN               0x1C
#define MVM_OC_JKILL                0x1D
#define MVM_OC_JSUSP                0x1E
#define MVM_OC_JWAKE                0x1F
#define MVM_OC_JSTART               0x20
#define MVM_OC_ACQ_LOCK             0x21
#define MVM_OC_REL_LOCK             0x22
#define MVM_OC_ACQ_LATCH            0x23
#define MVM_OC_REL_LATCH            0x24
#define MVM_OC_VMOVE                0x25
#define MVM_OC_DEC                  0x26
#define MVM_OC_TEMP_NAME            0x27

#define MODE_SHARED                 0x0000000000000000
#define MODE_EXCLUSIVE              0x0000000000000001

#define ACCESS_GLOBAL               0x0000000000000000

#define CONTAINER_PROGPOOL          0x0000000000000000

#define FORCE_NO_FORCE 0
#define FORCE_FORCE    1

#define VARIABLE_RAX        0
#define VARIABLE_RCX        1
#define VARIABLE_LOCAL_0    2
#define VARIABLE_LOCAL_1    3
#define VARIABLE_LOCAL_2    4

#define EXIT_CODE_COMMIT            0
#define EXIT_CODE_ABORT_BY_USER     1
#define EXIT_CODE_ABORT_BY_SYSTEM   2

#define ADD(/* MVM_OC_* */ oc, /* u64 */ op)                                                                           \
{                                                                                                                      \
    instruction_t inst = {                                                                                             \
        .opcode    = oc,                                                                                               \
        .operand   = op                                                                                                \
     };                                                                                                                \
    program_add(prog, &inst);                                                                                          \
}

#define BEGIN(/* program_t * */ out, /* const char * */ name, /* const char * */ author, /* const char * */ comment )  \
{                                                                                                                      \
    program_t *prog;                                                                                                   \
    program_new(&prog, name, author, comment);                                                                         \
    out = prog;

#define END()                                                                                                          \
}

int mondrian_vm_create(mondrian_vm_t **out, mondrian_t *db);

int mondrian_vm_run(mondrian_vm_t *vm, const program_t *program, int *return_value);

int mondrian_vm_reset(mondrian_vm_t *vm);

int mondrian_vm_free(mondrian_vm_t *vm);

int program_new(program_t **out, const char *prog_name, const char *prog_author, const char *prog_comment);

int program_add(program_t *out, instruction_t *inst);

int program_print(FILE *out, const program_t *program);

const char *program_name(const program_t *program);

program_t *program_cpy(const program_t *program);

int program_free(program_t *program);



