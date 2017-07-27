#include <mvm.h>
#include <debug.h>
#include <mondrian.h>

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

#define CHECKFOR_ELEMENT(container, element)                                                                           \
    CHECKFOR((container == CONTAINER_PROGPOOL && element != ACCESS_GLOBAL), MVM_EC_BADELEMENT,                         \
             "unknown element id in line '%d' in '%s': %lld",                                                          \
             (vm->pc - 1), program_name(vm->program), element)

#define OPERAND_STACK_POP()                                                                                            \
    *(u64*) vector_pop_unsafe(vm->operand_stack)

#define OPERAND_STACK_PUSH(value)                                                                                      \
{                                                                                                                      \
    CHECKFOR_STACKOVERFLOW()                                                                                           \
    vector_add_unsafe(vm->operand_stack, 1, &value);                                                                   \
}

typedef u64 operand_t;
typedef u8  opcode_t;

typedef enum shared_resource {
    SR_PROGPOOL
} shared_resource;

typedef enum access_mode {
    AM_EXCLUSIVE,
    AM_SHARED
} access_mode;

typedef struct shared_resource_release_state_t {
    access_mode mode;
    shared_resource resource;
    bool is_released;
} shared_resource_release_state_t;

typedef size_t lock_id_t;
typedef size_t latch_id_t;

struct mondrian_vm_t
{
    int pc;
    bool rollback;
    int return_value;
    int temp_name_counter;


    int error_code;
    char *error_details;

    vector_t *operand_stack;
    vector_t *locks;
    vector_t *latches;
    vector_t *variables;
    vector_t *temp_names;

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

    vector_t *instructions;

} program_t;

//----------------------------------------------------------------------------------------------------------------------

static inline int mondrian_vm_tick(mondrian_vm_t *vm);
static inline int mondrian_vm_rollback(mondrian_vm_t *vm);

static inline lock_id_t mondrian_vm_ackn_lock(mondrian_vm_t *vm, shared_resource target, access_mode mode);
static inline void mondrian_vm_set_var(mondrian_vm_t *vm, u64 index, u64 *value);


//----------------------------------------------------------------------------------------------------------------------

static int exec_abort(mondrian_vm_t *vm, u64 operand);
static int exec_addcol(mondrian_vm_t *vm, u64 operand);
static int exec_byname(mondrian_vm_t *vm, u64 operand);
static int exec_commit(mondrian_vm_t *vm, u64 operand);
static int exec_dclone(mondrian_vm_t *vm, u64 operand);
static int exec_dopen(mondrian_vm_t *vm, u64 operand);
static int exec_rjmp_nz(mondrian_vm_t *vm, u64 operand);
static int exec_fwrite(mondrian_vm_t *vm, u64 operand);
static int exec_push(mondrian_vm_t *vm, u64 operand);
static int exec_isset(mondrian_vm_t *vm, u64 operand);
static int exec_npop(mondrian_vm_t *vm, u64 operand);
static int exec_pop(mondrian_vm_t *vm, u64 operand);
static int exec_sleep(mondrian_vm_t *vm, u64 operand);
static int exec_swap(mondrian_vm_t *vm, u64 operand);
static int exec_tcreate(mondrian_vm_t *vm, u64 operand);
static int exec_tdrop(mondrian_vm_t *vm, u64 operand);
static int exec_tdup(mondrian_vm_t *vm, u64 operand);
static int exec_tinfo(mondrian_vm_t *vm, u64 operand);
static int exec_tinsert(mondrian_vm_t *vm, u64 operand);
static int exec_tinstall(mondrian_vm_t *vm, u64 operand);
static int exec_tlist(mondrian_vm_t *vm, u64 operand);
static int exec_progclean(mondrian_vm_t *vm, u64 operand);
static int exec_progninfo(mondrian_vm_t *vm, u64 operand);
static int exec_proglist(mondrian_vm_t *vm, u64 _);
static int exec_vload(mondrian_vm_t *vm, u64 operand);
static int exec_vstore(mondrian_vm_t *vm, u64 operand);
static int exec_jlist(mondrian_vm_t *vm, u64 operand);
static int exec_jinfo(mondrian_vm_t *vm, u64 operand);
static int exec_jclean(mondrian_vm_t *vm, u64 operand);
static int exec_jkill(mondrian_vm_t *vm, u64 operand);
static int exec_jsusp(mondrian_vm_t *vm, u64 operand);
static int exec_jwake(mondrian_vm_t *vm, u64 operand);
static int exec_jstart(mondrian_vm_t *vm, u64 operand);
static int exec_acq_lock(mondrian_vm_t *vm, u64 container);
static int exec_rel_lock(mondrian_vm_t *vm, u64 operand);
static int exec_acq_latch(mondrian_vm_t *vm, u64 operand);
static int exec_rel_latch(mondrian_vm_t *vm, u64 operand);
static int exec_vmove(mondrian_vm_t *vm, u64 index);
static int exec_dec(mondrian_vm_t *vm, u64 operand);
static int exec_temp_name(mondrian_vm_t *vm, u64 operand);


static struct {
    u8 opcode;
    const char mnemonic[256];
    bool operand_used;
    int (*function)(mondrian_vm_t *vm, u64 operand);
} mvm_opcode_register[] = {
        { MVM_OC_ABORT,     "abort",     false,     exec_abort },
        { MVM_OC_ADDCOL,    "addcol",    false,     exec_addcol },
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
        { MVM_OC_TCREATE,   "tcreate",   true,      exec_tcreate },
        { MVM_OC_TDROP,     "tdrop",     false,     exec_tdrop },
        { MVM_OC_TDUP,      "tdup",      false,     exec_tdup },
        { MVM_OC_TINFO,     "tinfo",     false,     exec_tinfo },
        { MVM_OC_TINSERT,   "tinsert",   false,     exec_tinsert },
        { MVM_OC_TINSTALL,  "tinstall",  false,     exec_tinstall },
        { MVM_OC_TLIST,     "tlist",     false,     exec_tlist },
        { MVM_OC_PROGCLEAN, "progclean", false,     exec_progclean },
        { MVM_OC_PROGINFO,  "proginfo",  false,     exec_progninfo },
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
        { MVM_OC_TEMP_NAME,  "temp_name", false,     exec_temp_name }
};

//----------------------------------------------------------------------------------------------------------------------

int mondrian_vm_create(mondrian_vm_t **out, mondrian_t *db)
{
    (*out) = malloc(sizeof(mondrian_vm_t));
    if (*out == NULL) {
        return MONDRIAN_ERROR;
    } else {
        **out = (mondrian_vm_t) {
            .pc = 0,
            .rollback = false,
            .return_value = 0,
            .temp_name_counter = 0,
            .operand_stack = vector_create(sizeof(operand_t), OPERAND_STACK_CAPACITY),
            .locks = vector_create(sizeof(shared_resource_release_state_t), 5),
            .latches = vector_create(sizeof(shared_resource_release_state_t), 5),
            .variables = vector_create(sizeof(u64), 10),
            .temp_names = vector_create(sizeof(char *), 10),
            .program = NULL,
            .db = db,
            .error_code = MVM_EC_NOERR,
            .error_details = NULL
        };
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
            "   - releasing %zu lock(s)\n"
            "   - releasing %zu latche(s)\n"
            "   - freeing %zuB on heap for operand stack\n"
            "   - freeing %zuB on heap for locks list\n"
            "   - freeing %zuB on heap for latches list\n"
            "   - freeing %zuB on heap for variables list\n"
            "   - freeing %zuB on heap for temporary table name(s)\n",
              program_name(vm->program), vm->program,
              vm->locks->num_elements,
              vm->latches->num_elements,
              vector_memused(vm->operand_stack),
              vector_memused(vm->locks),
              vector_memused(vm->latches),
              vector_memused(vm->variables),
              vector_memused__str(vm->temp_names));

    shared_resource_release_state_t *release_states = (shared_resource_release_state_t *)(vm->locks->data);
    for (lock_id_t id = 0; id < vm->locks->num_elements; id++) {
        if (!release_states[id].is_released) {
            switch (release_states[id].resource) {
                case SR_PROGPOOL:
                    switch (release_states[id].mode) {
                        case AM_EXCLUSIVE:
                            progpool_unlock_exclusive(mondrian_get_progpool(vm->db));
                        break;
                        case AM_SHARED:
                            // TODO
                        panic("not implemented %s", "AM_SHARED");
                        break;
                        default: panic("Unknown access mode: %d", release_states[id].mode);
                    }

                    break;
                default: panic("Unknown shared resource type: %d", release_states[id].resource);
            }
            LOG_DEBUG(vm->db, "auto released lock id %zu for mvm job '%s' (%p)\n",
                      id, program_name(vm->program), vm->program);
        }
    }

    release_states = (shared_resource_release_state_t *)(vm->latches->data);
    for (latch_id_t id = 0; id < vm->latches->num_elements; id++) {
        if (!release_states[id].is_released) {
            switch (release_states[id].resource) {
                case SR_PROGPOOL:
                    switch (release_states[id].mode) {
                        case AM_EXCLUSIVE:
                            // TODO
                            panic("not implemented %s", "AM_EXCLUSIVE");
                            break;
                        case AM_SHARED:
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

    vector_free(vm->operand_stack);
    vector_free(vm->locks);
    vector_free(vm->latches);
    vector_free(vm->variables);
    vector_free__str(vm->temp_names);

    vm->operand_stack = vm->locks = vm->latches = vm->variables = vm->temp_names = NULL;
    free(vm);

    return MONDRIAN_OK;
}

int program_new(program_t **out, const char *prog_name, const char *prog_author,
                const char *prog_comment)
{
    *out = malloc(sizeof(program_t));
    **out = (program_t) {
        .instructions = vector_create(sizeof(instruction_t), 10),
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
    vector_add(out->instructions, 1, inst);
    return MONDRIAN_OK;
}

int program_print(FILE *out, const program_t *program)
{
    if (out == NULL || program == NULL)
        return MONDRIAN_ERROR;

    size_t num_instructions = vector_num_elements(program->instructions);
    instruction_t *data = (instruction_t *) vector_get(program->instructions);

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

const char *program_name(const program_t *program)
{
    return program->prog_name;
}

program_t *program_cpy(const program_t *program)
{
    program_t *cpy = NULL;
    if(program_new(&cpy, program->prog_name, program->prog_author, program->prog_comments) == MONDRIAN_OK) {
        cpy->instructions = vector_cpy(program->instructions);
    }
    return cpy;
}

int program_free(program_t *program)
{
    vector_free(program->instructions);
    free(program);
    return MONDRIAN_OK;
}

//----------------------------------------------------------------------------------------------------------------------

static inline int mondrian_vm_tick(mondrian_vm_t *vm)
{
    assert (vm);
    assert (vm->operand_stack);
    if ((vm->pc >= 0) && (vm->pc < vm->program->instructions->num_elements)) {
        instruction_t *inst = vector_at(vm->program->instructions, vm->pc++);
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

static inline int mondrian_vm_rollback(mondrian_vm_t *vm)
{
    // TODO
    return MONDRIAN_OK;
}

static inline lock_id_t mondrian_vm_ackn_lock(mondrian_vm_t *vm, shared_resource target, access_mode mode)
{
    lock_id_t lock_id = vm->locks->num_elements;
    shared_resource_release_state_t state = {
        .mode = mode,
        .resource = target,
        .is_released = false
    };
    vector_add(vm->locks, 1, &state);
    return lock_id;
}

static inline void mondrian_vm_set_var(mondrian_vm_t *vm, u64 index, u64 *value)
{
    vector_set(vm->variables, index, 1, &value);
}

static int exec_abort(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_addcol(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_byname(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_commit(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_dclone(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_dopen(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_rjmp_nz(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_fwrite(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_push(mondrian_vm_t *vm, u64 operand)
{
    vector_add(vm->operand_stack, 1, &operand);
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
    return MONDRIAN_ERROR;
}

static int exec_sleep(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_swap(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_tcreate(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_tdrop(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_tdup(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_tinfo(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_tinsert(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_tinstall(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_tlist(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_progclean(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_progninfo(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_proglist(mondrian_vm_t *vm, u64 _)
{
    prog_id_t *list;
    s64 num_progs;
    progpool_list(&list, (size_t *) &num_progs, mondrian_get_progpool(vm->db));
    while (num_progs--) {
        OPERAND_STACK_PUSH(*list++);
    }
    OPERAND_STACK_PUSH(num_progs);
    return MONDRIAN_OK;
}

static int exec_vload(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_vstore(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
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

static int exec_rel_lock(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
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

static int exec_dec(mondrian_vm_t *vm, u64 operand)
{
    return MONDRIAN_ERROR;
}

static int exec_temp_name(mondrian_vm_t *vm, u64 operand)
{
    char buffer[256];
    sprintf(buffer, "TEMP_TABLE_#%d", vm->temp_name_counter++);
    char *temp_name = strdup(temp_name);
    vector_add(vm->temp_names, 1, &temp_name);
    OPERAND_STACK_PUSH(temp_name);
    return MONDRIAN_OK;
}