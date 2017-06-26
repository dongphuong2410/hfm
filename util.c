#include <stdio.h>
#include <stdlib.h>
#include <capstone/capstone.h>

#include "util.h"
#include "private.h"
#include "log.h"

uint64_t _find_addr_after_instruction(vmi_instance_t vmi, uint64_t start_v, char *mnemonic, char *ops);

uint64_t util_find_trampoline_addr(vmi_instance_t vmi)
{
    uint64_t trampoline_addr = 0;
    status_t status;
    uint64_t lstar = 0;
    uint8_t code[PAGE_SIZE] = {0};   /* Assume CALL is within first page */

    status = vmi_get_vcpureg(vmi, &lstar, MSR_LSTAR, 0);
    if (VMI_SUCCESS != status) {
        writelog(LV_ERROR, "Failed to get MSR_LSTAR address");
        goto done;
    }

    addr_t lstar_p = vmi_translate_kv2p(vmi, lstar);
    if (0 == lstar_p) {
        writelog(LV_ERROR, "Failed to translate virtual LSTAR to physical address");
        goto done;
    }

    /* Read kernel instructions into code */
    status = vmi_read_pa(vmi, lstar_p, code, sizeof(code));
    if (status < PAGE_SIZE) {
        writelog(LV_ERROR, "Failed to read instructions from 0x%lx\n", lstar_p);
        goto done;
    }

    /* Look for int 3 */
    for (int curr_inst = 0; curr_inst < PAGE_SIZE; curr_inst++) {
        if (code[curr_inst] == BREAKPOINT_INST) {
            trampoline_addr = lstar + curr_inst;
            break;
        }
    }
done:
    return trampoline_addr;
}

uint64_t util_find_return_addr(vmi_instance_t vmi)
{
    uint64_t lstar, return_point_addr = 0;
    status_t status = vmi_get_vcpureg(vmi, &lstar, MSR_LSTAR, 0);
    if (VMI_SUCCESS != status) {
        writelog(LV_ERROR, "Failed to get MSR_LSTAR address");
        goto done;
    }
    csh handle;
    cs_insn *inst;
    size_t count, offset = ~0;
    uint64_t ret = 0;
    uint8_t code[PAGE_SIZE];

    char *mnemonic = "call";
    char *ops = NULL;

    uint64_t start_p = vmi_translate_kv2p(vmi, lstar);
    if (0 == start_p) {
        writelog(LV_ERROR, "Failed to translate virtual start address to physical address");
        goto done;
    }

    /* Read kernel instructions into code */
    status = vmi_read_pa(vmi, start_p, code, sizeof(code));
    if (VMI_FAILURE == status) {
        writelog(LV_ERROR, "Failed to read instructions from 0x%lx", start_p);
        goto done;
    }
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        writelog(LV_ERROR, "Failed to open capstone");
        goto done;
    }
    /* Find CALL inst, and note address of inst, which follows */
    count = cs_disasm(handle, code, sizeof(code), 0, 0, &inst);
    if (count > 0) {
        size_t i;
        for (i = 0; i < count; i++) {
                //printf("%s  0x%lx\n", inst[i].mnemonic, lstar + inst[i+1].address);
            if (0 == strcmp(inst[i].mnemonic, mnemonic)
                    && (NULL == ops || 0 == strcmp(inst[i].op_str, ops))) {
                //printf("%s  0x%lx\n", inst[i].mnemonic, lstar + inst[i+1].address);
                offset = inst[i + 1].address;
            }
        }
        cs_free(inst, count);
    } else {
        writelog(LV_ERROR, "Failed to disassemble system-call handler");
        goto done;
    }
    if (~0 == offset) {
        writelog(LV_ERROR, "Did not find call in system-call handler");
        goto done;
    }
    cs_close(&handle);
    ret = lstar + offset;
done:
    return ret;
}
