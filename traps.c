#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <libvmi/slat.h>

#include "log.h"
#include "libhfm.h"


hfm_status_t hfm_register_trap(vmhdlr_t *handler, const char *func_name)
{
    hfm_status_t ret = SUCCESS;
    addr_t func_addr;
    vmi_pause_vm(handler->vmi);

    /* Find vaddr of syscall */
    func_addr = vmi_translate_ksym2v(handler->vmi, func_name);
    if (0 == func_addr) {
        writelog(LV_WARN, "Counldn't locate the address of kernel symbol '%s'", func_name);
        goto done;
    }

    hfm_inject_trap(handler, func_addr);
done:
    vmi_resume_vm(handler->vmi);
    return ret;
}

void hfm_destroy_traps(vmhdlr_t *handler)
{
    hfm_delete_trap(handler);
}
