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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

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

typedef struct __attribute__((__packed__)) instruction_t
{
    u8  opcode;
    u64 operand;
} instruction_t;

typedef enum access_mode {
    MODE_SHARED               = 0x0000000000000000,
    MODE_EXCLUSIVE            = 0x0000000000000001
} access_mode;

// ---------------------------------------------------------------------------------------------------------------------
// C O N S T A N T S
// ---------------------------------------------------------------------------------------------------------------------

#define MVM_OC_ABORT                0x00
#define MVM_OC_ADDATTR              0x01
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
#define MVM_OC_FCREATE              0x0E
#define MVM_OC_FDROP                0x0F
#define MVM_OC_FDUP                 0x10
#define MVM_OC_FINFO                0x11
#define MVM_OC_FINSERT              0x12
#define MVM_OC_FINSTALL             0x13
#define MVM_OC_FLIST                0x14
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
#define MVM_OC_OFIELD               0x28
#define MVM_OC_WFIELD               0x29
#define MVM_OC_PRINT                0x2A

#define ACCESS_GLOBAL               0x0000000000000000

#define CONTAINER_PROGPOOL          0x0000000000000000

#define FORCE_NO_FORCE 0
#define FORCE_FORCE    1

#define VARIABLE_RAX        0
#define VARIABLE_RCX        1
#define VARIABLE_RTC        2
#define VARIABLE_RFC        3
#define VARIABLE_LOCAL_0    4
#define VARIABLE_LOCAL_1    5
#define VARIABLE_LOCAL_2    6

#define FRAGMENT_SCOPE_GLOBAL   0x0000000000000001
#define FRAGMENT_SCOPE_LOCAL    0x0000000000000002

#define FRAGMENT_HOST_PLAIN_COLUMN_STORE       0x0000000000000000
#define FRAGMENT_HOST_PLAIN_ROW_STORE          0x0000000000000001

static const char STRING_BUILTIN_PROG_ID[]     = "Program ID";
static const char STRING_BUILTIN_NAME[]        = "Name";
static const char STRING_BUILTIN_AUTHOR[]      = "Author";
static const char STRING_BUILTIN_COMMENT[]     = "Comment";
static const char STRING_BUILTIN_SIZE[]        = "Size";

#define EXIT_CODE_COMMIT            0
#define EXIT_CODE_ABORT_BY_USER     1
#define EXIT_CODE_ABORT_BY_SYSTEM   2

#define PROGRAM_MAX_NAME_LENGTH         256
#define PROGRAM_MAX_AUTHOR_LENGTH       256
#define PROGRAM_MAX_COMMENT_LENGTH     1024

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

int mondrian_vm_create(mondrian_vm_t **out, mondrian_t *db);

int mondrian_vm_run(mondrian_vm_t *vm, const program_t *program, int *return_value);

int mondrian_vm_reset(mondrian_vm_t *vm);

int mondrian_vm_free(mondrian_vm_t *vm);

int program_new(program_t **out, const char *prog_name, const char *prog_author, const char *prog_comment);

int program_add(program_t *out, instruction_t *inst);

int program_print(FILE *out, const program_t *program);

size_t program_sizeof(const program_t *program);

const char *program_name(const program_t *program);

program_t *program_cpy(const program_t *program);

int program_free(program_t *program);



