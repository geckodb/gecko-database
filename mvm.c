#include <mvm.h>
#include <debug.h>
#include <mondrian.h>

#define STRING_CAST(opperand) (const char *) opperand

typedef struct vm_frame_t
{
    int return_value;
    vector_t *operand_stack;
} vm_frame_t;

struct mondrian_vm_t
{
    int pc;
    bool rollback;
    int return_value;
    vector_t *frames;
    const program_t *program;
    mondrian_t *db;
};

typedef struct program_t
{
    vector_t *instructions;
} program_t;

//----------------------------------------------------------------------------------------------------------------------

static inline const char *opcode_to_mnemonic(u32 opcode);

static inline int mondrian_vm_tick(mondrian_vm_t *vm);
static inline int mondrian_vm_exec_print(u64 op1);

static inline int mondrian_vm_exec_create_table(mondrian_vm_t *vm, u64 op1);
static inline int mondrian_vm_exec_end_table(mondrian_vm_t *vm);
static inline int mondrian_vm_exec_add_column(mondrian_vm_t *vm, u64 op1, u64 op2, u64 op3, u64 op4);

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
            .frames = vector_create(sizeof(vm_frame_t), 5),
            .program = NULL,
            .db = db
        };
        return ((*out)->frames == NULL ? MONDRIAN_ERROR : MONDRIAN_OK);
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
        assert (vm->frames->num_elements == 0);

        vm->program = program;
        vm_frame_t frame = {
            .operand_stack = vector_create(sizeof(u64), 32),
            .return_value = 0,
        };
        vector_add(vm->frames, 1, &frame);
        while (mondrian_vm_tick(vm));
        LOG_DEBUG(vm->db, "mvm program %p finished with exit code %d", program, vm->return_value);
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
    if (vm->frames->num_elements != 0) {
        return MONDRIAN_ERROR;
    } else {
        vector_free(vm->frames);
        free(vm);
        return MONDRIAN_OK;
    }
}

int program_new(program_t **out)
{
    *out = malloc(sizeof(program_t));
    **out = (program_t) {
        .instructions = vector_create(sizeof(instruction_t), 10)
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
    printf("# dump of mondrian vm code for program %p\n", program);
    printf("# -----------------------------------------------------------------------------------\n");
    for (size_t i = 0; i < num_instructions; i++) {
        instruction_t inst = data[i];
        const char *mnemonic = opcode_to_mnemonic(inst.opcode);
        printf("0x%08lx\t0x%04x\t%-15s%-10llu    %-10llu    %-10llu    %-10llu\n",
               (i * sizeof(instruction_t)), inst.opcode,
               mnemonic, inst.op1, inst.op2, inst.op3, inst.op4);
    }
    return MONDRIAN_OK;
}

int program_free(program_t *program)
{
    vector_free(program->instructions);
    free(program);
    return MONDRIAN_OK;
}

//----------------------------------------------------------------------------------------------------------------------

static inline const char *opcode_to_mnemonic(u32 opcode)
{
    for (size_t j = 0; j < ARRAY_LEN_OF(opcode_mnemonics); j++) {
        if (opcode_mnemonics[j].opcode == opcode) {
            return opcode_mnemonics[j].mnemonic;
        }
    }
    return "(Unknown)";
}


static inline int mondrian_vm_tick(mondrian_vm_t *vm)
{
    assert (vm);
    assert (vm->frames);
    assert (vm->frames->num_elements > 0);
//    vm_frame_t *frame = vector_peek(vm->frames);
    int exec_result = MONDRIAN_OK;
    if ((vm->pc >= 0) && (vm->pc < vm->program->instructions->num_elements)) {
        instruction_t *inst = vector_at(vm->program->instructions, vm->pc);
        vm->pc += (vm->rollback ? -1 : +1);
        switch (inst->opcode) {
            case MONDRIAN_OPCODE_ABORT:
                LOG_DEBUG(vm->db, "ABORT  mvm program %p ABORT received", vm->program);
                vm->return_value = MONDRIAN_VM_USER_ABORT;
                return MONDRIAN_BREAK;
            case MONDRIAN_OPCODE_COMMIT:
                LOG_DEBUG(vm->db, "COMMIT mvm program %p COMMIT received", vm->program);
                vm->return_value = MONDRIAN_VM_COMMIT;
                return MONDRIAN_BREAK;
            case MONDRIAN_OPCODE_PRINT:
                mondrian_vm_exec_print(inst->op1);
                break;
            case MONDRIAN_OPCODE_CREATE_TABLE:
                exec_result = mondrian_vm_exec_create_table(vm, inst->op1);
                break;
            case MONDRIAN_OPCODE_END_TABLE:
                exec_result = mondrian_vm_exec_end_table(vm);
                break;
            case MONDRIAN_OPCODE_ADD_COLUMN:
                exec_result = mondrian_vm_exec_add_column(vm, inst->op1, inst->op2, inst->op3, inst->op4);
                break;
            default:
            panic("Unknown opcode in program %p: 0x%04x", vm->program, inst->opcode);
        }
        if ((exec_result) != MONDRIAN_OK) {
            LOG_DEBUG(vm->db, "ABORT  0x%08lx: %s in mvm program %p rejected. Initialize ROLLBACK.",
                      (vm->pc - 1) * sizeof(instruction_t), opcode_to_mnemonic(inst->opcode), vm->program);
            vm->rollback = true;
            vm->pc--;
        }
        return MONDRIAN_CONTINUE;
    } else {
        if (vm->rollback && vm->pc == -1) {
            vm->return_value = MONDRIAN_VM_SYSTEM_ABORT;
            return MONDRIAN_BREAK;
        } else {
            perror("program counter exceeds program length");
            return MONDRIAN_ERROR;
        }
    }
}

static inline int mondrian_vm_exec_print(u64 op1)
{
    printf("%s", STRING_CAST(op1));
    return MONDRIAN_OK;
}

static inline int mondrian_vm_exec_create_table(mondrian_vm_t *vm, u64 op1)
{
    if (vm->rollback) {
        LOG_DEBUG(vm->db, "ROLLBACK CreateTable in %p", vm);
        return MONDRIAN_OK;
    } else {
        return MONDRIAN_OK;
    }
}

static inline int mondrian_vm_exec_end_table(mondrian_vm_t *vm)
{
    if (vm->rollback) {
        LOG_DEBUG(vm->db, "ROLLBACK EndTable in %p", vm);
        return MONDRIAN_OK;
    } else {
        return MONDRIAN_ERROR;
    }
}

static inline int mondrian_vm_exec_add_column(mondrian_vm_t *vm, u64 op1, u64 op2, u64 op3, u64 op4)
{
    if (vm->rollback) {
        LOG_DEBUG(vm->db, "ROLLBACK AddColumn in %p", vm);
        return MONDRIAN_OK;
    } else {
        return MONDRIAN_OK;
    }
}