#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <libvmi/slat.h>

#include "strace.h"
#include "log.h"
#include "xen_helper.h"

static void _setup_mem_trap(vmhdlr_t *handler, addr_t va);

uint8_t trap = 0xCC;

hfm_status_t strace_register(vmhdlr_t *handler, const char *func_name)
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

    _setup_mem_trap(handler, func_addr);
done:
    vmi_resume_vm(handler->vmi);
    return ret;
}

static void _setup_mem_trap(vmhdlr_t *handler, addr_t va)
{
    printf("Setup memtrap\n");
    status_t status;
    vmi_pause_vm(handler->vmi);
    addr_t pa = vmi_translate_kv2p(handler->vmi, va);
    if (0 == pa) {
        writelog(LV_DEBUG, "Virtual addr translation failed: %lx", va);
        goto done;
    }
    addr_t frame = pa >> PAGE_OFFSET_BITS;
    addr_t shadow_offset = pa % PAGE_SIZE;

    /* Setup and activate shadow view */
    uint64_t proposed_memsize = handler->memsize + PAGE_SIZE;
    handler->remapped = xen_alloc_shadow_frame(handler->xen, proposed_memsize);
    if (handler->remapped == 0) {
        writelog(LV_DEBUG, "Extend memory failed for shadow page");
        goto done;
    }
    handler->memsize = proposed_memsize;    //Update current memsize after extend

    /* Change altp2m_idx view to map to new remapped address */
    status = vmi_slat_change_gfn(handler->vmi, handler->altp2m_idx, frame, handler->remapped);
    if (VMI_SUCCESS != status) {
        writelog(LV_DEBUG, "Failed to update altp2m_idx view to new remapped address");
        goto done;
    }

    /* Copy original page to remapped page */
    uint8_t buff[PAGE_SIZE] = {0};
    size_t ret = vmi_read_pa(handler->vmi, frame << PAGE_OFFSET_BITS, buff, PAGE_SIZE);
    if (PAGE_SIZE != ret) {
        writelog(LV_DEBUG, "Failed to read in syscall page");
        goto done;
    }

    ret = vmi_write_pa(handler->vmi, handler->remapped << PAGE_OFFSET_BITS, buff, PAGE_SIZE);
    if (PAGE_SIZE != ret) {
        writelog(LV_DEBUG, "Failed to write to remapped page");
        goto done;
    }

    /* Establish callback on a R/W of this page */
    //vmi_set_mem_event(handler->vmi, frame, VMI_MEMACCESS_RW, handler->altp2m_idx);

    addr_t rpa = (handler->remapped << PAGE_OFFSET_BITS) + pa % PAGE_SIZE;
    status = vmi_write_8_pa(handler->vmi, rpa, &trap);
    if (VMI_SUCCESS != status) {
        writelog(LV_DEBUG, "Failed to write interrupt to shadow page");
        goto done;
    }

done:
    vmi_resume_vm(handler->vmi);
}

void strace_destroy(vmhdlr_t *handler)
{
    xen_free_shadow_frame(handler->xen, &handler->remapped);
}
