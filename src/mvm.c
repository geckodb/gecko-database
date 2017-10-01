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

#include <mvm.h>
#include <debug.h>
#include <mondrian.h>
#include <schema.h>
#include <frag.h>
#include <tuplet_field.h>

#ifndef OPERAND_STACK_CAPACITY
#define OPERAND_STACK_CAPACITY  1024
#endif

#define MVM_MAGIC_WORD     0x4D564D
#define MVM_MAJOR_VERSION  1
#define MVM_MINOR_VERSION  1
#define STRING_CAST(opperand) (const char *) opperand

#define MVM_EC_NOERR                0
#define MVM_EC_STACKUNDERFLOW       1
#define MVM_EC_ILLEGALARG           2
#define MVM_EC_BADCONTAINER         3
#define MVM_EC_BADELEMENT           4
#define MVM_EC_STACKOVERFLOW        5
#define MVM_EC_BADSCOPE             6
#define MVM_EC_FRAGMENT_EXISTS      7
#define MVM_EC_UNKNOWNATTRTYPE      8
#define MVM_EC_BADFIELDLEN          9
#define MVM_EC_BADCOLUMNFLAGS      10
#define MVM_EC_BADFRAGMENTTYPE     11
#define MVM_EC_NOFRAGMENT          12

#define CHECKFOR(expr, errcode, msg, ...)                                                                              \
{                                                                                                                      \
    if (expr) {                                                                                                        \
        vm->error_code = errcode;                                                                                      \
        char msg_buffer[1024];                                                                                         \
        sprintf(msg_buffer, msg, __VA_ARGS__);                                                                         \
        vm->error_details = strdup(msg_buffer);                                                                        \
        return MONDRIAN_ERROR;                                                                                         \
    }                                                                                                                  \
}

#define CHECKFOR_STACKOVERFLOW()                                                                                       \
    CHECKFOR(vm->operand_stack->num_elements + 1 >= vm->operand_stack->element_capacity, MVM_EC_STACKOVERFLOW,         \
             "instruction at line '%d' in '%s' failed: operand stack overflow",                                        \
             (vm->pc - 1), program_name(vm->program))

#define CHECKFOR_STACKUNDERFLOW(required_num_operands)                                                                 \
    CHECKFOR(vm->operand_stack->num_elements < required_num_operands, MVM_EC_STACKUNDERFLOW,                           \
             "instruction at line '%d' in '%s' failed: not enough elements in operand stack",                          \
             (vm->pc - 1), program_name(vm->program))

#define CHECKFOR_LOCKLATCH_MODE(mode)                                                                                  \
    CHECKFOR(mode != MODE_SHARED && mode != MODE_EXCLUSIVE, MVM_EC_ILLEGALARG,                                         \
             "lock/latch mode definition in line '%d' in '%s' is unknown",                                             \
             (vm->pc - 1), program_name(vm->program))

#define CHECKFOR_CONTAINER(container)                                                                                  \
    CHECKFOR(container != CONTAINER_PROGPOOL, MVM_EC_BADCONTAINER,                                                     \
             "container id in line '%d' in '%s': %lld",                                                                \
             (vm->pc - 1), program_name(vm->program), container)

#define CHECKFOR_SCOPE(scope)                                                                                          \
    CHECKFOR(scope != FRAGMENT_SCOPE_GLOBAL && scope != FRAGMENT_SCOPE_LOCAL, MVM_EC_BADSCOPE,                         \
             "bad scope in line '%d' in '%s': %lld",                                                                   \
             (vm->pc - 1), program_name(vm->program), scope)


#define CHECKFOR_ELEMENT(container, element)                                                                           \
    CHECKFOR((container == CONTAINER_PROGPOOL && element != ACCESS_GLOBAL), MVM_EC_BADELEMENT,                         \
             "unknown element id in line '%d' in '%s': %lld",                                                          \
             (vm->pc - 1), program_name(vm->program), element)

#define CHECKFOR_UNIQUE_LOCAL_FRAGMENT_NAME(frag_name)                                                                 \
    CHECKFOR(!is_unique_fragment_name(vm, frag_name), MVM_EC_FRAGMENT_EXISTS,                                          \
             "instruction at line '%d' in '%s' failed: local fragment name '%s' already exists",                       \
             (vm->pc - 1), program_name(vm->program), frag_name)

#define CHECKFOR_LOCAL_FRAGID(frag_id)                                                                                 \
    CHECKFOR((frag_id >= vm->fragments->num_elements ||                                                                \
             ((local_fragment_t *)vec_at(vm->fragments, frag_id))->is_dropped),                                        \
             MVM_EC_NOFRAGMENT,                                                                                        \
             "instruction at line '%d' in '%s' failed: no such fragment in local space: %lld",                         \
             (vm->pc - 1), program_name(vm->program), frag_id)

#define CHECKFOR_TYPE(type)                                                                                            \
    CHECKFOR((type != FT_BOOL && type != FT_INT8 && type != FT_INT16 && type != FT_INT32 && type != FT_INT64 &&        \
              type != FT_UINT8 && type != FT_UINT16 && type != FT_UINT32 && type != FT_UINT64 &&                       \
              type != FT_FLOAT32 && type != FT_FLOAT64 && type != FT_CHAR ) , MVM_EC_UNKNOWNATTRTYPE,                  \
             "instruction at line '%d' in '%s' failed: unknown attribute data type '%lld'",                            \
             (vm->pc - 1), program_name(vm->program), type)

#define CHECKFOR_REP(rep)                                                                                              \
    CHECKFOR(rep <= 0, MVM_EC_BADFIELDLEN,                                                                             \
             "instruction at line '%d' in '%s' failed: bad field length '%lld'",                                       \
             (vm->pc - 1), program_name(vm->program), rep)

#define CHECKFOR_FRAGMENTTYPE(fragment_type)                                                                           \
    CHECKFOR((fragment_type != FRAGMENT_HOST_PLAIN_COLUMN_STORE && fragment_type != FRAGMENT_HOST_PLAIN_ROW_STORE),    \
             MVM_EC_BADFRAGMENTTYPE,                                                                                   \
             "instruction at line '%d' in '%s' failed: bad fragment type '%lld'",                                      \
             (vm->pc - 1), program_name(vm->program), fragment_type)

#define CHECKFOR_NOTZERO(value)                                                                                        \
    CHECKFOR((value == 0), MVM_EC_BADFRAGMENTTYPE,                                                                     \
             "instruction at line '%d' in '%s' failed: value is not allowed to be zero",                               \
             (vm->pc - 1), program_name(vm->program))


#define CHECKFOR_FLAGS(flags)                                                                                          \
    CHECKFOR(!((flags == FLAG_REGULAR) ||                                                                              \
                                                                                                                       \
             ((IS_FLAG_SET(flags, FLAG_PRIMARY) ||                                                                     \
              IS_FLAG_SET(flags, FLAG_FOREIGN) ||                                                                      \
              IS_FLAG_SET(flags, FLAG_NULLABLE) ||                                                                     \
              IS_FLAG_SET(flags, FLAG_AUTOINC) ||                                                                      \
              IS_FLAG_SET(flags, FLAG_UNIQUE))    &&                                                                   \
                                                                                                                       \
              (IS_FLAG_SET(flags, FLAG_PRIMARY) && !(IS_FLAG_SET(flags, FLAG_FOREIGN) ||                               \
                                                   IS_FLAG_SET(flags ,FLAG_NULLABLE)) ||                               \
              IS_FLAG_SET(flags, FLAG_FOREIGN) && !(IS_FLAG_SET(flags, FLAG_PRIMARY) ||                                \
                                                   IS_FLAG_SET(flags, FLAG_NULLABLE)) ||                               \
              IS_FLAG_SET(flags, FLAG_NULLABLE) && !(IS_FLAG_SET(flags, FLAG_PRIMARY) ||                               \
                                                    IS_FLAG_SET(flags, FLAG_FOREIGN) ||                                \
                                                    IS_FLAG_SET(flags, FLAG_AUTOINC)) ||                               \
              IS_FLAG_SET(flags, FLAG_AUTOINC) && !(IS_FLAG_SET(flags, FLAG_NULLABLE) ||                               \
                                                   IS_FLAG_SET(flags, FLAG_FOREIGN))))),                               \
             MVM_EC_BADCOLUMNFLAGS,                                                                                    \
             "instruction at line '%d' in '%s' failed: bad column flag combination '%lld'",                            \
             (vm->pc - 1), program_name(vm->program), flags)


#define OPERAND_STACK_POP()                                                                                            \
    *(u64*) vec_pop_unsafe(vm->operand_stack)

#define OPERAND_STACK_PEEK()                                                                                           \
    *(u64*) vec_peek_unsafe(vm->operand_stack)

#define OPERAND_STACK_PUSH(value)                                                                                      \
{                                                                                                                      \
    CHECKFOR_STACKOVERFLOW()                                                                                           \
    vec_add_unsafe(vm->operand_stack, 1, &value);                                                                      \
}

typedef u64 operand_t;
typedef u64 frag_id_t;
typedef u8  opcode_t;

typedef struct frag_handle_t {
    u64 is_global   : 1;
    u64 frag_id : 63;
} frag_handle_t;

typedef enum shared_resource {
    SR_PROGPOOL
} shared_resource;

typedef struct shared_resource_release_state_t {
    access_mode mode;
    shared_resource resource;
    bool is_released;
} shared_resource_release_state_t;

typedef size_t lock_id_t;
typedef size_t latch_id_t;

typedef struct variable_entry_t {
    bool initialized;
    u64 value;
} variable_entry_t;

static variable_entry_t EMPTY_ENTRY = {
    .initialized = false,
    .value = 0
};

typedef struct local_fragment_t {
    frag_t *fragment;
    bool is_dropped;
} local_fragment_t;

struct mondrian_vm_t
{
    int pc;
    bool rollback;
    int return_value;
    int temp_name_counter;


    int error_code;
    char *error_details;

    vec_t *operand_stack;
    vec_t *locks;
    vec_t *latches;
    vec_t *variables;
    vec_t *temp_names;
    vec_t *fragments;

    const program_t *program;
    mondrian_t *db;
    clock_t start;
};

typedef struct program_t
{
    struct {
        u64 magic_word        : 48;
        u64 mvm_version_minor :  5;
        u64 mvm_version_major : 11;
    };
    char *prog_name;
    char *prog_author;
    char *prog_comments;

    vec_t *instructions;

} program_t;

//----------------------------------------------------------------------------------------------------------------------

 int mondrian_vm_tick(mondrian_vm_t *vm);
 int mondrian_vm_rollback(mondrian_vm_t *vm);

 lock_id_t mondrian_vm_ackn_lock(mondrian_vm_t *vm, shared_resource target, access_mode mode);
 int mondrian_vm_release_lock(mondrian_vm_t *vm, lock_id_t lock_id);

 void mondrian_vm_set_var(mondrian_vm_t *vm, u64 index, u64 *value);
 int mondrian_vm_get_var(u64 *out, mondrian_vm_t *vm, u64 index);
 int mondrian_vm_install_frag_local(frag_id_t *out, mondrian_vm_t *vm, frag_t *frag);
 frag_t *mondrian_vm_get_frag_local(mondrian_vm_t *vm, frag_id_t id);

//----------------------------------------------------------------------------------------------------------------------

 bool is_unique_fragment_name(mondrian_vm_t *vm, const char *name);
 int drop_fragment_with_name(mondrian_vm_t *vm, const char *name);
 void frag_handle_make(frag_handle_t *out, u64 scope, frag_id_t frag_id);
 bool frag_handle_is_global(frag_handle_t *handle);
 frag_id_t frag_handle_get_frag_id(frag_handle_t *handle);

//----------------------------------------------------------------------------------------------------------------------

static int exec_abort(mondrian_vm_t *vm, u64 operand);
static int exec_addattr(mondrian_vm_t *vm, u64 operand);
static int exec_byname(mondrian_vm_t *vm, u64 operand);
static int exec_commit(mondrian_vm_t *vm, u64 operand);
static int exec_dclone(mondrian_vm_t *vm, u64 operand);
static int exec_dopen(mondrian_vm_t *vm, u64 operand);
static int exec_rjmp_nz(mondrian_vm_t *vm, u64 pos);
static int exec_fwrite(mondrian_vm_t *vm, u64 operand);
static int exec_push(mondrian_vm_t *vm, u64 operand);
static int exec_isset(mondrian_vm_t *vm, u64 operand);
static int exec_npop(mondrian_vm_t *vm, u64 operand);
static int exec_pop(mondrian_vm_t *vm, u64 operand);
static int exec_sleep(mondrian_vm_t *vm, u64 operand);
static int exec_swap(mondrian_vm_t *vm, u64 operand);
static int exec_fcreate(mondrian_vm_t *vm, u64 force);
static int exec_fdrop(mondrian_vm_t *vm, u64 operand);
static int exec_fdup(mondrian_vm_t *vm, u64 operand);
static int exec_finfo(mondrian_vm_t *vm, u64 operand);
static int exec_finsert(mondrian_vm_t *vm, u64 operand);
static int exec_finstall(mondrian_vm_t *vm, u64 _);
static int exec_tlist(mondrian_vm_t *vm, u64 operand);
static int exec_progclean(mondrian_vm_t *vm, u64 operand);
static int exec_proginfo(mondrian_vm_t *vm, u64 _);
static int exec_proglist(mondrian_vm_t *vm, u64 _);
static int exec_vload(mondrian_vm_t *vm, u64 variable_idx);
static int exec_vstore(mondrian_vm_t *vm, u64 variable_idx);
static int exec_jlist(mondrian_vm_t *vm, u64 operand);
static int exec_jinfo(mondrian_vm_t *vm, u64 operand);
static int exec_jclean(mondrian_vm_t *vm, u64 operand);
static int exec_jkill(mondrian_vm_t *vm, u64 operand);
static int exec_jsusp(mondrian_vm_t *vm, u64 operand);
static int exec_jwake(mondrian_vm_t *vm, u64 operand);
static int exec_jstart(mondrian_vm_t *vm, u64 operand);
static int exec_acq_lock(mondrian_vm_t *vm, u64 container);
static int exec_rel_lock(mondrian_vm_t *vm, u64 _);
static int exec_acq_latch(mondrian_vm_t *vm, u64 operand);
static int exec_rel_latch(mondrian_vm_t *vm, u64 operand);
static int exec_vmove(mondrian_vm_t *vm, u64 index);
static int exec_dec(mondrian_vm_t *vm, u64 var_idx);
static int exec_temp_name(mondrian_vm_t *vm, u64 operand);
static int exec_ofield(mondrian_vm_t *vm, u64 _);
static int exec_wfield(mondrian_vm_t *vm, u64 _);
static int exec_print(mondrian_vm_t *vm, u64 _);


static struct {
    u8 opcode;
    const char mnemonic[256];
    bool operand_used;
    int (*function)(mondrian_vm_t *vm, u64 operand);
} mvm_opcode_register[] = {
        { MVM_OC_ABORT,     "abort",     false,     exec_abort },
        { MVM_OC_ADDATTR,   "addattr",   false,     exec_addattr },
        { MVM_OC_BYNAME,    "byname",    false,     exec_byname },
        { MVM_OC_COMMIT,    "commit",    false,     exec_commit },
        { MVM_OC_DCLONE,    "dclone",    false,     exec_dclone },
        { MVM_OC_DOPEN,     "dopen",     true,      exec_dopen },
        { MVM_OC_RJMP_NZ,   "rjmp_nz",   true,      exec_rjmp_nz },
        { MVM_OC_FWRITE,    "fwrite",    false,     exec_fwrite },
        { MVM_OC_PUSH,      "push",      true,      exec_push },
        { MVM_OC_ISSET,     "isset",     true,      exec_isset },
        { MVM_OC_NPOP,      "npop",      false,     exec_npop },
        { MVM_OC_POP,       "pop",       false,     exec_pop },
        { MVM_OC_SLEEP,     "sleep",     true,      exec_sleep },
        { MVM_OC_SWAP,      "swap",      false,     exec_swap },
        { MVM_OC_FCREATE,   "fcreate",   true,      exec_fcreate },
        { MVM_OC_FDROP,     "fdrop",     false,     exec_fdrop },
        { MVM_OC_FDUP,      "fdup",      false,     exec_fdup },
        { MVM_OC_FINFO,     "finfo",     false,     exec_finfo },
        { MVM_OC_FINSERT,   "finsert",   false,     exec_finsert },
        { MVM_OC_FINSTALL,  "finstall",  false,     exec_finstall },
        { MVM_OC_FLIST,     "flist",     false,     exec_tlist },
        { MVM_OC_PROGCLEAN, "progclean", false,     exec_progclean },
        { MVM_OC_PROGINFO,  "proginfo",  false,     exec_proginfo },
        { MVM_OC_PROGLIST,  "proglist",  false,     exec_proglist },
        { MVM_OC_VLOAD,     "vload",     true,      exec_vload },
        { MVM_OC_VSTORE,    "vstore",    true,      exec_vstore },

        { MVM_OC_JLIST,     "jlist",     true,      exec_jlist },
        { MVM_OC_JINFO,     "jinfo",     false,     exec_jinfo },
        { MVM_OC_JCLEAN,    "jclean",    false,     exec_jclean },
        { MVM_OC_JKILL,     "jkill",     false,     exec_jkill },
        { MVM_OC_JSUSP,     "jsusp",     false,     exec_jsusp },
        { MVM_OC_JWAKE,     "jwake",     false,     exec_jwake },
        { MVM_OC_JSTART,    "jstart",    false,     exec_jstart },

        { MVM_OC_ACQ_LOCK,   "acq_lock",  true,      exec_acq_lock },
        { MVM_OC_REL_LOCK,   "rel_lock",  false,     exec_rel_lock },
        { MVM_OC_ACQ_LATCH,  "acq_latch", true,      exec_acq_latch },
        { MVM_OC_REL_LATCH,  "rel_latch", false,     exec_rel_latch },
        { MVM_OC_VMOVE,      "vmove",     true,      exec_vmove },
        { MVM_OC_DEC,        "dec",       true,      exec_dec },
        { MVM_OC_TEMP_NAME,  "temp_name", false,     exec_temp_name },
        { MVM_OC_OFIELD,     "ofield",    false,     exec_ofield },
        { MVM_OC_WFIELD,     "wfield",    false,     exec_wfield },
        { MVM_OC_PRINT,      "print",     false,     exec_print }
};

//----------------------------------------------------------------------------------------------------------------------

int mondrian_vm_create(mondrian_vm_t **out, mondrian_t *db)
{
    (*out) = GS_REQUIRE_MALLOC(sizeof(mondrian_vm_t));
    if (*out == NULL) {
        return MONDRIAN_ERROR;
    } else {
        **out = (mondrian_vm_t) {
            .pc = 0,
            .rollback = false,
            .return_value = 0,
            .temp_name_counter = 0,
            .operand_stack = vec_new(sizeof(operand_t), OPERAND_STACK_CAPACITY),
            .locks = vec_new(sizeof(shared_resource_release_state_t), 5),
            .latches = vec_new(sizeof(shared_resource_release_state_t), 5),
            .variables = vec_new(sizeof(variable_entry_t), 10),
            .temp_names = vec_new(sizeof(char *), 10),
            .fragments = vec_new(sizeof(local_fragment_t), 10),
            .program = NULL,
            .db = db,
            .error_code = MVM_EC_NOERR,
            .error_details = NULL
        };

        if ((*out)->variables) {
            vec_memset((*out)->variables, 0, (*out)->variables->element_capacity, &EMPTY_ENTRY);
        }

        return ((*out)->operand_stack != NULL && (*out)->locks != NULL && (*out)->latches != NULL &&
                (*out)->variables != NULL && (*out)->temp_names != NULL) ?
                MONDRIAN_OK : MONDRIAN_ERROR;
    }
}

int mondrian_vm_run(mondrian_vm_t *vm, const program_t *program, int *return_value)
{
    if (vm == NULL || program == NULL || return_value == NULL)
        return MONDRIAN_ERROR;
    else {
        assert (vm->pc == 0);
        assert (vm->program == NULL);
        assert (vm->return_value == 0);
        assert (vm->operand_stack);
        assert (vm->operand_stack->num_elements == 0);

        vm->program = program;
        vm->start = clock();
        while (mondrian_vm_tick(vm));
        LOG_DEBUG(vm->db, "mvm job '%s' (%p) finished after %0.2fmsec with exit code %d", program_name(program),
                  program, (double)(clock() - vm->start) / CLOCKS_PER_SEC * 1000, vm->return_value);
        *return_value = vm->return_value;
        return MONDRIAN_OK;
    }
}

int mondrian_vm_reset(mondrian_vm_t *vm)
{
    if (!vm) {
        return MONDRIAN_ERROR;
    } else {
        vm->pc = 0;
        vm->return_value = 0;
        vm->program = NULL;
        return MONDRIAN_OK;
    }
}

int mondrian_vm_free(mondrian_vm_t *vm)
{
    assert (vm);
    assert (vm->operand_stack);
    assert (vm->locks);
    assert (vm->latches);
    assert (vm->variables);
    assert (vm->temp_names);

    LOG_DEBUG(vm->db, "cleanup mvm job '%s' (%p)\n"
            "   - releasing up to %zu lock(s)\n"
            "   - releasing up to %zu latche(s)\n"
            "   - freeing %zuB on heap for operand stack\n"
            "   - freeing %zuB on heap for locks list\n"
            "   - freeing %zuB on heap for latches list\n"
            "   - freeing %zuB on heap for variables list\n"
            "   - freeing %zuB on heap for local fragment(s)\n"
            "   - freeing %zuB on heap for temporary string(s)\n",
              program_name(vm->program), vm->program,
              vm->locks->num_elements,
              vm->latches->num_elements,
              vec_memused(vm->operand_stack),
              vec_memused(vm->locks),
              vec_memused(vm->latches),
              vec_memused(vm->variables),
              vec_memused(vm->fragments),
              vec_memused__str(vm->temp_names));

    for (lock_id_t id = 0; id < vm->locks->num_elements; id++) {
        mondrian_vm_release_lock(vm, id);
    }

    shared_resource_release_state_t *release_states = (shared_resource_release_state_t *)(vm->latches->data);
    for (latch_id_t id = 0; id < vm->latches->num_elements; id++) {
        if (!release_states[id].is_released) {
            switch (release_states[id].resource) {
                case SR_PROGPOOL:
                    switch (release_states[id].mode) {
                        case MODE_EXCLUSIVE:
                            // TODO
                            panic("not implemented %s", "AM_EXCLUSIVE");
                            break;
                        case MODE_SHARED:
                            // TODO
                            panic("not implemented %s", "AM_SHARED");
                            break;
                        default: panic("Unknown access mode: %d", release_states[id].mode);
                    }

                    break;
                default: panic("Unknown shared resource type: %d", release_states[id].resource);
            }
            LOG_DEBUG(vm->db, "auto released latch id %zu for mvm job '%s' (%p)\n",
                      id, program_name(vm->program), vm->program);
        }
    }

    vec_free(vm->operand_stack);
    vec_free(vm->locks);
    vec_free(vm->latches);
    vec_free(vm->variables);
    vec_free__str(vm->temp_names);

    vm->operand_stack = vm->locks = vm->latches = vm->variables = vm->temp_names = NULL;
    free(vm);

    return MONDRIAN_OK;
}

int program_new(program_t **out, const char *prog_name, const char *prog_author,
                const char *prog_comment)
{
    if (strlen(prog_name) + 1 >= PROGRAM_MAX_NAME_LENGTH || strlen(prog_author) + 1 >= PROGRAM_MAX_AUTHOR_LENGTH ||
        strlen(prog_comment) + 1 >= PROGRAM_MAX_COMMENT_LENGTH) {
        return MONDRIAN_ERROR;
    }

    *out = GS_REQUIRE_MALLOC(sizeof(program_t));
    **out = (program_t) {
        .instructions = vec_new(sizeof(instruction_t), 10),
        .magic_word   = MVM_MAGIC_WORD,
        .mvm_version_major = MVM_MAJOR_VERSION,
        .mvm_version_minor = MVM_MINOR_VERSION,
        .prog_name = strdup(prog_name),
        .prog_author = strdup(prog_author),
        .prog_comments = strdup(prog_comment)
    };
    return ((*out)->instructions != NULL ? MONDRIAN_OK : MONDRIAN_ERROR);
}

int program_add(program_t *out, instruction_t *inst)
{
    assert(out);
    assert(inst);
    vec_pushback(out->instructions, 1, inst);
    return MONDRIAN_OK;
}

int program_print(FILE *out, const program_t *program)
{
    if (out == NULL || program == NULL)
        return MONDRIAN_ERROR;

    size_t num_instructions = vec_length(program->instructions);
    instruction_t *data = (instruction_t *) vec_data(program->instructions);

    printf("# -----------------------------------------------------------------------------------\n");
    printf("# mvm bytecode of '%s' (%p)\n", program_name(program), program);
    printf("# -----------------------------------------------------------------------------------\n");
    for (size_t i = 0; i < num_instructions; i++) {
        instruction_t inst = data[i];
        panic_if((inst.opcode >= ARRAY_LEN_OF(mvm_opcode_register)), "internal error: opcode 0x%02x exceeds register.",
                  inst.opcode);
        const char *mnemonic = mvm_opcode_register[inst.opcode].mnemonic;
        if (mvm_opcode_register[inst.opcode].operand_used) {
            printf("%-4ld 0x%02x\t%-15s0x%016llx\n",
                   (i * sizeof(instruction_t)), inst.opcode, mnemonic, inst.operand);
        } else {
            printf("%-4ld 0x%02x\t%-15s\n",
                   (i * sizeof(instruction_t)), inst.opcode, mnemonic);
        }
    }
    return MONDRIAN_OK;
}

size_t program_sizeof(const program_t *program)
{
    return (program == NULL ? 0 : vec_sizeof(program->instructions));
}

const char *program_name(const program_t *program)
{
    return program->prog_name;
}

program_t *program_cpy(const program_t *program)
{
    program_t *cpy = NULL;
    if(program_new(&cpy, program->prog_name, program->prog_author, program->prog_comments) == MONDRIAN_OK) {
        cpy->instructions = vec_cpy_deep(program->instructions);
    }
    return cpy;
}

int program_free(program_t *program)
{
    vec_free(program->instructions);
    free(program);
    return MONDRIAN_OK;
}

//----------------------------------------------------------------------------------------------------------------------

 int mondrian_vm_tick(mondrian_vm_t *vm)
{
    assert (vm);
    assert (vm->operand_stack);
    if ((vm->pc >= 0) && (vm->pc < vm->program->instructions->num_elements)) {
        instruction_t *inst = vec_at(vm->program->instructions, vm->pc++);
        panic_if((inst->opcode > ARRAY_LEN_OF(mvm_opcode_register)),
                 "internal error: opcode '0x%02x' exceeds register.", inst->opcode);
        if((mvm_opcode_register[inst->opcode].function(vm, inst->operand)) != MONDRIAN_OK) {
            const char *prog_name = program_name(vm->program);
            LOG_DEBUG(vm->db, "*** ABORT BY SYSTEM ***  %ld: %s in mvm job '%s' (%p): %s",
                      (vm->pc - 1) * sizeof(instruction_t), mvm_opcode_register[inst->opcode].mnemonic,
                      prog_name, vm->program, vm->error_details);
            panic_if((mondrian_vm_rollback(vm) != MONDRIAN_OK),
                      "*** FATAL *** rollback failed for mvm job '%s' (%p)", prog_name, vm->program);
            vm->return_value = EXIT_CODE_ABORT_BY_SYSTEM;
            return MONDRIAN_ERROR;
        } else {
            u8 opcode = mvm_opcode_register[inst->opcode].opcode;
            vm->return_value = (opcode == MVM_OC_COMMIT) ? EXIT_CODE_COMMIT :
                               ((opcode == MVM_OC_ABORT) ? EXIT_CODE_ABORT_BY_USER : -1);
            return (opcode == MVM_OC_COMMIT || opcode == MVM_OC_ABORT) ? MONDRIAN_BREAK : MONDRIAN_CONTINUE;
        }
    } else {
        perror("program counter exceeds program length");
        return MONDRIAN_ERROR;
    }
}

 int mondrian_vm_rollback(mondrian_vm_t *vm)
{
    // TODO
    return MONDRIAN_OK;
}

 lock_id_t mondrian_vm_ackn_lock(mondrian_vm_t *vm, shared_resource target, access_mode mode)
{
    lock_id_t lock_id = vm->locks->num_elements;
    shared_resource_release_state_t state = {
        .mode = mode,
        .resource = target,
        .is_released = false
    };
    vec_pushback(vm->locks, 1, &state);
    return lock_id;
}

 int mondrian_vm_release_lock(mondrian_vm_t *vm, lock_id_t lock_id)
{
    shared_resource_release_state_t *release_states = (shared_resource_release_state_t *)(vm->locks->data);
    if (lock_id >= vm->locks->num_elements) {
        return MONDRIAN_ERROR;
    } else {
        if (!release_states[lock_id].is_released) {
            switch (release_states[lock_id].resource) {
                case SR_PROGPOOL:
                    switch (release_states[lock_id].mode) {
                        case MODE_EXCLUSIVE:
                            progpool_unlock_exclusive(mondrian_get_progpool(vm->db));
                            release_states[lock_id].is_released = true;
                            break;
                        case MODE_SHARED:
                            // TODO
                        panic("not implemented %s", "AM_SHARED");
                            break;
                        default: panic("Unknown access mode: %d", release_states[lock_id].mode);
                    }

                    break;
                default: panic("Unknown shared resource type: %d", release_states[lock_id].resource);
            }
            LOG_DEBUG(vm->db, "released lock id %zu for mvm job '%s' (%p)\n",
                      lock_id, program_name(vm->program), vm->program);
            return MONDRIAN_OK;
        } return MONDRIAN_ALREADY_DONE;
    }
}

 void mondrian_vm_set_var(mondrian_vm_t *vm, u64 index, u64 *value)
{
    variable_entry_t entry = {
        .initialized = true,
        .value = *value
    };
    size_t vec_elem = vm->variables->num_elements;
    if (vec_elem + 1 >= vm->variables->element_capacity) {
        size_t new_vec_cap = 2 * vec_elem;
        vec_resize(vm->variables, new_vec_cap);
        vec_memset(vm->variables, vec_elem, new_vec_cap, &EMPTY_ENTRY);
    }
    vec_set(vm->variables, index, 1, &entry);
}

 int mondrian_vm_get_var(u64 *out, mondrian_vm_t *vm, u64 index)
{
    if (vm == NULL || out == NULL) {
        return MONDRIAN_ERROR;
    } else {
        const variable_entry_t *entry = (const variable_entry_t *) vec_at(vm->variables, index);
        if (entry != NULL && entry->initialized) {
            *out = entry->value;
            return MONDRIAN_OK;
        } else return MONDRIAN_ERROR;
    }
}

 int mondrian_vm_install_frag_local(frag_id_t *out, mondrian_vm_t *vm, frag_t *frag)
{
    local_fragment_t local = {
        .is_dropped = false,
        .fragment = frag
    };
    *out = vm->fragments->num_elements;
    vec_pushback(vm->fragments, 1, &local);
    return MONDRIAN_OK;
}

 frag_t *mondrian_vm_get_frag_local(mondrian_vm_t *vm, frag_id_t id)
{
    const local_fragment_t *local = (const local_fragment_t *) vec_at(vm->fragments, id);
    return (local == NULL ? NULL : local->fragment);
}

//----------------------------------------------------------------------------------------------------------------------

 bool is_unique_fragment_name(mondrian_vm_t *vm, const char *name)
{
    local_fragment_t *fragments = (local_fragment_t *) vm->fragments->data;
    for (size_t i = 0; i < vm->fragments->num_elements; i++) {
        if ((!fragments[i].is_dropped) && (!strcmp(frag_schema(fragments[i].fragment)->frag_name, name))) {
            return false;
        }
    }
    return true;
}

 int drop_fragment_with_name(mondrian_vm_t *vm, const char *name)
{
    local_fragment_t *fragments = (local_fragment_t *) vm->fragments->data;
    for (size_t i = 0; i < vm->fragments->num_elements; i++) {
        if (!strcmp(frag_schema(fragments[i].fragment)->frag_name, name)) {
            if (fragments[i].is_dropped) {
                return MONDRIAN_ERROR;
            } else {
                fragments[i].is_dropped = true;
                return MONDRIAN_OK;
            }
        }
    }
    return MONDRIAN_NOSUCHELEM;
}

 void frag_handle_make(frag_handle_t *out, u64 scope, frag_id_t frag_id)
{
    out->is_global = (scope == FRAGMENT_SCOPE_GLOBAL);
    out->frag_id   = frag_id;
}

 bool frag_handle_is_global(frag_handle_t *handle)
{
    return handle->is_global;
}

 frag_id_t frag_handle_get_frag_id(frag_handle_t *handle)
{
    return handle->frag_id;
}

//----------------------------------------------------------------------------------------------------------------------

static int exec_abort(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_addattr(mondrian_vm_t *vm, u64 operand)
{
    CHECKFOR_STACKUNDERFLOW(5)
    const char *name = (const char *) OPERAND_STACK_POP();
    u64 type         = OPERAND_STACK_POP();
    u64 rep          = OPERAND_STACK_POP();
    u64 flags        = OPERAND_STACK_POP();
    schema_t *schema = (schema_t *) OPERAND_STACK_PEEK();

    if (schema == NULL || schema_attr_by_name(schema, name) != NULL) {
        return MONDRIAN_ERROR;
    }

    CHECKFOR_TYPE(type);
    CHECKFOR_REP(rep);
    CHECKFOR_FLAGS(flags);

    if (IS_FLAG_SET(flags, FLAG_PRIMARY)) {
        flags |= FLAG_UNIQUE;
    }

    attr_id_t attr_id = attr_create(name, type, rep, schema);
    OPERAND_STACK_PUSH(attr_id);
    return MONDRIAN_OK;
}

static int exec_byname(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_commit(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_OK;
}

static int exec_dclone(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_dopen(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_rjmp_nz(mondrian_vm_t *vm, u64 pos)
{
    s64 jmp = *((s64 *)&pos);
    u64 test;
    mondrian_vm_get_var(&test, vm, VARIABLE_RCX);
    if (test != 0) {
        int new_pc = (--vm->pc) + jmp;
        if (new_pc < 0 || new_pc >= vm->program->instructions->num_elements) {
            return MONDRIAN_ERROR;
        } else {
            vm->pc = new_pc;
            return MONDRIAN_OK;
        }
    } else return MONDRIAN_OK;
}

static int exec_fwrite(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_push(mondrian_vm_t *vm, u64 operand)
{
    vec_pushback(vm->operand_stack, 1, &operand);
    return MONDRIAN_OK;
}

static int exec_isset(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_npop(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_pop(mondrian_vm_t *vm, u64 operand)
{
    CHECKFOR_STACKUNDERFLOW(1)
    OPERAND_STACK_POP();
    return MONDRIAN_OK;
}

static int exec_sleep(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_swap(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_fcreate(mondrian_vm_t *vm, u64 force)
{
    CHECKFOR_STACKUNDERFLOW(2)
    u64 scope = OPERAND_STACK_POP();
    const char *name = (const char *) OPERAND_STACK_POP();
    CHECKFOR_SCOPE(scope)

    schema_t *schema;

    switch (scope) {
        case FRAGMENT_SCOPE_LOCAL:
            if (!force) {
                CHECKFOR_UNIQUE_LOCAL_FRAGMENT_NAME(name);
            } else {
                drop_fragment_with_name(vm, name);
            }
            schema = schema_new(name);
            OPERAND_STACK_PUSH(schema);
            return MONDRIAN_OK;
        case FRAGMENT_SCOPE_GLOBAL:
            panic("Not implemented! %s", "FRAGMENT_SCOPE_GLOBAL");
            return MONDRIAN_ERROR;
        default: panic("Unknown scope %llu", scope);
    }
    return MONDRIAN_ERROR;
}

static int exec_fdrop(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_fdup(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_finfo(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_finsert(mondrian_vm_t *vm, u64 operand)
{
    CHECKFOR_STACKUNDERFLOW(2)
    u64 num             = OPERAND_STACK_POP();
    frag_handle_t frag_handle = *((frag_handle_t*) &OPERAND_STACK_POP());
    CHECKFOR_NOTZERO(num)

    if (frag_handle_is_global(&frag_handle)) {
        panic("Not implemented: '%s'", "global frag handle");
    } else {
        CHECKFOR_LOCAL_FRAGID(frag_handle_get_frag_id(&frag_handle));
        frag_t *frag = mondrian_vm_get_frag_local(vm, frag_handle.frag_id);
        if (frag == NULL) {
            return MONDRIAN_ERROR;
        }
        tuplet_t *tuplet_ptr = GS_REQUIRE_MALLOC(num * sizeof(tuplet_t *));
        frag_insert(tuplet_ptr, frag, num);
        mondrian_vm_set_var(vm, VARIABLE_RTC, (u64 *)&tuplet_ptr);
        return MONDRIAN_OK;
    }
}

static int exec_finstall(mondrian_vm_t *vm, u64 _)
{
    CHECKFOR_STACKUNDERFLOW(3)
    u64 scope        = OPERAND_STACK_POP();
    u64 frag_type    = OPERAND_STACK_POP();
    u64 capacity     = OPERAND_STACK_POP();
    schema_t *schema = (schema_t *) OPERAND_STACK_POP();
    CHECKFOR_SCOPE(scope)
    CHECKFOR_FRAGMENTTYPE(frag_type)
    CHECKFOR_NOTZERO(capacity)
    enum frag_impl_type_t impl_type;
    switch (frag_type) {
        case FRAGMENT_HOST_PLAIN_ROW_STORE:
            impl_type = FIT_HOST_NSM_VM;
            break;
        case FRAGMENT_HOST_PLAIN_COLUMN_STORE:
            impl_type = FIT_HOST_DSM_VM;
            break;
        default:
            panic("Unknown fragment type '%lld'", frag_type);
    }

    frag_id_t frag_id;
    frag_t *frag = frag_new(schema, capacity, impl_type);

    switch (scope) {
        case FRAGMENT_SCOPE_GLOBAL:
            panic("Not implemented: '%s'", "FRAGMENT_SCOPE_GLOBAL");
            break;
        case FRAGMENT_SCOPE_LOCAL:
            mondrian_vm_install_frag_local(&frag_id, vm, frag);
            break;
        default:
            panic("Unknown scope: %lld", scope);
    }

    frag_handle_t handle;
    frag_handle_make(&handle, scope, frag_id);

    OPERAND_STACK_PUSH(handle);
    return MONDRIAN_OK;
}

static int exec_tlist(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_progclean(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_proginfo(mondrian_vm_t *vm, u64 _)
{
    CHECKFOR_STACKUNDERFLOW(1);
    prog_id_t prog_id = OPERAND_STACK_POP();
    const program_t *program = progpool_get(mondrian_get_progpool(vm->db), prog_id);
    if (program == NULL) {
        return MONDRIAN_ERROR;
    } else {
        size_t size = program_sizeof(program);
        OPERAND_STACK_PUSH(size);
        OPERAND_STACK_PUSH(program->prog_comments);
        OPERAND_STACK_PUSH(program->prog_author);
        OPERAND_STACK_PUSH(program->prog_name);
        OPERAND_STACK_PUSH(prog_id);
        return MONDRIAN_OK;
    }
}

static int exec_proglist(mondrian_vm_t *vm, u64 _)
{
    prog_id_t *list;
    s64 num_progs, counter;
    progpool_list(&list, (size_t *) &counter, mondrian_get_progpool(vm->db));
    num_progs = counter;
    while (counter--) {
        OPERAND_STACK_PUSH(*list++);
    }
    OPERAND_STACK_PUSH(num_progs);
    return MONDRIAN_OK;
}

static int exec_vload(mondrian_vm_t *vm, u64 variable_idx)
{
    u64 value;
    int result = mondrian_vm_get_var(&value, vm, variable_idx);
    if (result == MONDRIAN_OK) {
        OPERAND_STACK_PUSH(value);
        return MONDRIAN_OK;
    } else return MONDRIAN_ERROR;
}

static int exec_vstore(mondrian_vm_t *vm, u64 variable_idx)
{
    CHECKFOR_STACKUNDERFLOW(1)
    u64 value = OPERAND_STACK_PEEK();
    mondrian_vm_set_var(vm, variable_idx, &value);
    return MONDRIAN_OK;
}

static int exec_jlist(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_jinfo(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_jclean(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_jkill(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_jsusp(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_jwake(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_jstart(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_acq_lock(mondrian_vm_t *vm, u64 container)
{
    CHECKFOR_STACKUNDERFLOW(2)
    u64 mode      = OPERAND_STACK_POP();
    u64 element   = OPERAND_STACK_POP();
    CHECKFOR_LOCKLATCH_MODE(mode)
    CHECKFOR_CONTAINER(container)
    CHECKFOR_ELEMENT(container, element)

    lock_id_t lock_id;

    switch (container) {
        case CONTAINER_PROGPOOL:
            switch (mode) {
                case MODE_EXCLUSIVE:
                    progpool_lock_exclusive(mondrian_get_progpool(vm->db));
                    break;
                case MODE_SHARED:
                    panic("Not implemented '%s'", "MODE_SHARED");
                    break;
                default:
                    panic("Unknown lock mode in vm %p", vm);
            }
            lock_id = mondrian_vm_ackn_lock(vm, SR_PROGPOOL, mode);
            break;
    }

    OPERAND_STACK_PUSH(lock_id);

    return MONDRIAN_OK;
}

static int exec_rel_lock(mondrian_vm_t *vm, u64 _)
{
    CHECKFOR_STACKUNDERFLOW(1)
    lock_id_t lock_id = OPERAND_STACK_POP();
    return (mondrian_vm_release_lock(vm, lock_id) != MONDRIAN_ERROR ? MONDRIAN_OK : MONDRIAN_ERROR);
}

static int exec_acq_latch(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_rel_latch(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_vmove(mondrian_vm_t *vm, u64 index)
{
    CHECKFOR_STACKUNDERFLOW(1)
    u64 value = OPERAND_STACK_POP();
    mondrian_vm_set_var(vm, index, &value);
    return MONDRIAN_OK;
}

static int exec_dec(mondrian_vm_t *vm, u64 var_idx)
{
    u64 value;
    mondrian_vm_get_var(&value, vm, var_idx);
    value--;
    mondrian_vm_set_var(vm, var_idx, &(value));
    return MONDRIAN_OK;
}

static int exec_temp_name(mondrian_vm_t *vm, u64 operand)
{
    char buffer[256];
    sprintf(buffer, "%s@%p:#%d", program_name(vm->program), vm->program, vm->temp_name_counter++);
    char *temp_name = strdup(buffer);
    vec_pushback(vm->temp_names, 1, &temp_name);
    OPERAND_STACK_PUSH(temp_name);
    return MONDRIAN_OK;
}

static int exec_ofield(mondrian_vm_t *vm, u64 _)
{
    tuplet_field_t *field;
    u64 mem_addr;
    mondrian_vm_get_var(&mem_addr, vm, VARIABLE_RTC);
//    tuplet_t *tuplet = (tuplet_t *) mem_addr;
    //field = tuplet_field_open(tuplet);
    mondrian_vm_set_var(vm, VARIABLE_RFC, (u64 *) &field);
    panic("Not implemented '%s'!", "exec_ofield");
    return MONDRIAN_OK;
}

static int exec_wfield(mondrian_vm_t *vm, u64 _)
{
    CHECKFOR_STACKUNDERFLOW(1)
    u64 mem_addr;
    mondrian_vm_get_var(&mem_addr, vm, VARIABLE_RFC);
    tuplet_field_t *cursor = (tuplet_field_t *) mem_addr;
    u64 data = OPERAND_STACK_POP();
    tuplet_field_write(cursor, &data, true);

    return MONDRIAN_OK;
}

static int exec_print(mondrian_vm_t *vm, u64 _)
{
    CHECKFOR_STACKUNDERFLOW(1)
    frag_handle_t *handle = (frag_handle_t *) &OPERAND_STACK_POP();
    if (handle->is_global) {
        panic("Not implemented: '%s'", "exec_print for global");
    } else {
        frag_t *frag = mondrian_vm_get_frag_local(vm, handle->frag_id);
        if (frag == NULL) {
            return MONDRIAN_ERROR;
        } else {
            frag_print(stdout, frag, 0, frag->ntuplets);
            return MONDRIAN_OK;
        }
    }
}