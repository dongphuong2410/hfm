#include <stdio.h>
#include <stdlib.h>

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
