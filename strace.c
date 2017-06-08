#include "strace.h"
#include "log.h"

hfm_status_t strace_register(vmhdlr_t *handler, const char *func_name)
{
    hfm_status_t ret = SUCCESS;
    addr_t func_addr;
    vmi_pause_vm(handler->vmi);
    func_addr = vmi_translate_ksym2v(handler->vmi, func_name);
    if (0 == func_addr) {
        writelog(LV_WARN, "Counldn't locate the address of kernel symbol '%s'", func_name);
        goto done;
    }
    printf("Address of %s : %lu\n", func_name, func_addr);
done:
    vmi_resume_vm(handler->vmi);
    return ret;
}
