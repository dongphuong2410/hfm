#include "file_created.h"
#include "hfm.h"
#include "log.h"

int syscall_cb(vmhdlr_t *handler, trap_data_t *data);
int sysret_cb(vmhdlr_t *handler, trap_data_t *data);

hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtCreateFile", syscall_cb, sysret_cb);
    return FAIL;
}

int syscall_cb(vmhdlr_t *handler, trap_data_t *data)
{
    return 1;
}

int sysret_cb(vmhdlr_t *handler, trap_data_t *data)
{
    addr_t attr = 0;
    if (handler->pm == VMI_PM_IA32E)
        attr = data->regs->r8;
    else {
        vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
        vmi_read_32_va(vmi, data->regs->rsp + sizeof(uint32_t) * 3, 0, (uint32_t *)&attr);
        hfm_release_vmi(handler);
    }
    return 0;
}
