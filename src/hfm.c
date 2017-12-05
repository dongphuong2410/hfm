#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <libvmi/slat.h>
#include <glib.h>

#include "hfm.h"
#include "xen_helper.h"
#include "config.h"
#include "log.h"
#include "trapmngr.h"
#include "output_format.h"
#include "win.h"
#include "constants.h"
#include "policy.h"
#include "libmon.h"

/**
* hfm maintains two page tables (two views), first page table (ORIGINAL_IDX) maps the kernel
* with no modification. The second (altp2m_idx, shadow table) adds breakpoints to the kernel
*
* ORIGIN_IDX is activated :
* (1) For a single instruction after trapping a read (such a read is likely the result of Kernel
*  Patch Protection. This allows to hide the present of hfm from other checking software
* (2) For a single instruction after trapping a guest trace-emplaced breakpoint. This allows
* the kernel to execute as expected after servicing the breakpoint.
*
* ALTP2M_IDX is activate : following a single-step execution, this restore the breakpoints after condition
* (1) or (2)
*/

#define ORIGIN_IDX 0

typedef struct memcb_pass_t {
    vmhdlr_t *handler;
    remapped_t *remapped;
    vmi_mem_access_t access;
    GSList *traps;
} memcb_pass_t;

static event_response_t _int3_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _pre_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _post_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _singlestep_cb(vmi_instance_t vmi, vmi_event_t *event);
void _remove_int3(vmhdlr_t *handler, addr_t pa);

static hfm_status_t _inject_trap(vmhdlr_t *handler, trap_t *trap);
static hfm_status_t _init_vmi(vmhdlr_t *handler);
static void _close_vmi(vmhdlr_t *handler);
static hfm_status_t _setup_altp2m(vmhdlr_t *handler);
static void _reset_altp2m(vmhdlr_t *handler);
static void _destroy_traps(vmhdlr_t *handler);
static uint64_t _create_shadow_page(vmhdlr_t *handler, uint64_t original_gfn);

extern config_t *config;
uint8_t INT3_CHAR = 0xCC;

hfm_status_t hfm_init(vmhdlr_t *handler)
{
    /* Init xen interface*/
    if ((handler->xen = xen_init_interface(handler->name)) == NULL) {
        writelog(handler->logid, LV_ERROR, "Failed to init XEN on domain %s", handler->name);
        xen_free_interface(handler->xen);
        goto error1;
    }

    char rekall_profile[1024];
    sprintf(rekall_profile, "%s/%s.json", config_get_str(config, "rekall-base"), handler->name);
    win_fill_offsets(rekall_profile, handler->offsets);
    win_fill_sizes(rekall_profile, handler->sizes);

    /* Init LibVMI */
    if (SUCCESS != _init_vmi(handler)) {
        goto error2;
    }
    /* Create altp2m view */
    if (SUCCESS != _setup_altp2m(handler)) {
        goto error3;
    }
    /* Init trap manager */
    handler->trap_manager = tm_init();
    if (handler->trap_manager == NULL) {
        writelog(handler->logid, LV_ERROR, "Failed to init trap manager on domain %s", handler->name);
        goto error4;
    }
    //TODO: Move output plugin to the outside (main.c),
    //Each vm should have one output seperatedly
    /* Init output plugin */
    char *output = config_get_str(config, "output");
    if (0 == strncmp(output, "csv", STR_BUFF)) {
        char out_file[1024];
        char *hfm_base = config_get_str(config, "csv-path");
        if (hfm_base == NULL) hfm_base = "./";
        sprintf(out_file, "%s/%s/%s.out", hfm_base, "output", handler->name);
        handler->out = out_init(OUT_CSV, out_file);
    }
    else if (0 == strncmp(output, "es", STR_BUFF)) {
        handler->out = out_init(OUT_ELASTICSEARCH, config_get_str(config, "output_es_url"), handler->name);
    }
    else {
        handler->out = out_init(OUT_CONSOLE);
    }
    if (!handler->out) {
        goto error5;
    }

    return SUCCESS;
error5:
    tm_destroy(handler->trap_manager);
error4:
    _reset_altp2m(handler);
error3:
    _close_vmi(handler);
error2:
    xen_free_interface(handler->xen);
error1:
    return FAIL;
}

void hfm_close(vmhdlr_t *handler)
{
    writelog(handler->logid, LV_INFO, "Close LibVMI on domain %s", handler->name);
    vmi_pause_vm(handler->vmi);

    /* Close output plugin */
    out_close(handler->out);

    /* Destroy all traps */
    _destroy_traps(handler);

    /* Reset altp2m view */
    _reset_altp2m(handler);

    /* Close xen interface */
    xen_free_interface(handler->xen);

    vmi_resume_vm(handler->vmi);

    /* Close LibVMI */
    _close_vmi(handler);
}

void hfm_listen(vmhdlr_t *handler)
{
    status_t status = vmi_events_listen(handler->vmi, 500);
}

hfm_status_t hfm_monitor_syscall(vmhdlr_t *handler, const char *func_name, cb_t sys_cb, cb_t ret_cb)
{
    hfm_status_t ret = SUCCESS;
    addr_t func_addr;
    vmi_pause_vm(handler->vmi);

    /* Translate syscall name into physical address */
    func_addr = vmi_translate_ksym2v(handler->vmi, func_name);
    if (0 == func_addr) {
        writelog(handler->logid, LV_WARN, "Counldn't locate the address of kernel symbol '%s'", func_name);
        goto done;
    }

    addr_t pa = vmi_translate_kv2p(handler->vmi, func_addr);
    if (0 == pa) {
        writelog(handler->logid, LV_DEBUG, "Virtual addr translation failed: %lx", func_addr);
        goto done;
    }
    //Create a trap
    trap_t *trap = (trap_t *)calloc(1, sizeof(trap_t));
    strncpy(trap->name, func_name, STR_BUFF);
    trap->cb = sys_cb;
    trap->pa = pa;
    trap->ret_cb = ret_cb;

    //Inject trap at physical address
    _inject_trap(handler, trap);
done:
    vmi_resume_vm(handler->vmi);
    return ret;
}

static event_response_t _int3_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    event_response_t rsp = 0;
    vmhdlr_t *handler = event->data;
    handler->regs[event->vcpu_id] = event->x86_regs;

    /* Calculate pa of interrupt event */
    addr_t pa = (event->interrupt_event.gfn << 12)
                    + event->interrupt_event.offset + event->interrupt_event.insn_length - 1;
    writelog(handler->logid, LV_DEBUG, "INT3 event vCPU %u pa %lx", event->vcpu_id, pa);

    /* Looking for traps registered at this pa */
    GSList *int3traps = tm_int3traps_at_pa(handler->trap_manager, pa);
    if (!int3traps) {
        /* No trap is currently registered for this location
           but this event may have been triggered by one we just removed */
        uint8_t test = 0;
        if (VMI_FAILURE == vmi_read_8_pa(handler->vmi, pa, &test)) {
            writelog(handler->logid, LV_ERROR, "Critical error in int3 callback, can't read page");
            handler->interrupted = -1;
            return 0;
        }
        if (test == INT3_CHAR) {
            event->interrupt_event.reinject = 1;
        }
        else {
            event->interrupt_event.reinject = 0;
        }
        return 0;
    }

    int8_t doubletrap = tm_check_doubletrap(handler->trap_manager, pa);
    if (doubletrap)
        event->interrupt_event.reinject = 1;
    else
        event->interrupt_event.reinject = 0;
    GSList *loop = int3traps;
    context_t *context = (context_t *)calloc(1, sizeof(context_t));
    context->regs = event->x86_regs;
    context->access_ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    context->access_ctx.dtb = event->x86_regs->cr3;
    context->hdlr = handler;
    while (loop) {
        trap_t *trap = loop->data;
        if (trap->cb) {
            context->trap = trap;

            void *extra = trap->cb(handler, context);
            if (extra && trap->ret_cb) {
                access_context_t ctx;
                uint64_t ret;
                ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
                ctx.dtb = event->x86_regs->cr3;
                ctx.addr = event->x86_regs->rsp;
                vmi_read_addr(handler->vmi, &ctx, &ret);
                trap_t *ret_trap = (trap_t *)calloc(1, sizeof(trap_t));
                ret_trap->pa = vmi_pagetable_lookup(handler->vmi, ctx.dtb, ret);
                ret_trap->cb = trap->ret_cb;
                ret_trap->self_destroy = 1;
                ret_trap->ret_cb = NULL;
                ret_trap->extra = extra;
                sprintf(ret_trap->name, "%s_return", trap->name);

                _inject_trap(handler, ret_trap);
            }
        }
        if (trap->self_destroy) {
            int trap_remains = tm_remove_int3trap(handler->trap_manager, trap);
            if (trap_remains == 0) {
                _remove_int3(handler, trap->pa);
            }
            free(trap);
        }
        loop = loop->next;
    }
    free(context);

    event->slat_id = ORIGIN_IDX;
    handler->step_event[event->vcpu_id]->callback = _singlestep_cb;
    handler->step_event[event->vcpu_id]->data = handler;
    rsp = rsp |
            VMI_EVENT_RESPONSE_TOGGLE_SINGLESTEP |
            VMI_EVENT_RESPONSE_VMM_PAGETABLE_ID;
    return rsp;
}

static event_response_t _pre_mem_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    vmhdlr_t *handler = event->data;
    handler->regs[event->vcpu_id] = event->x86_regs;

    //Generate data to pass to the _post_mem_cb
    memtrap_t *memw = tm_find_memtrap(handler->trap_manager, event->mem_event.gfn);
    if (!memw) {
        writelog(handler->logid, LV_DEBUG, "Event has been cleared for GFN 0x%lx but we are still in view %u\n",
                        event->mem_event.gfn, event->slat_id);
        return 0;
    }
    memcb_pass_t *pass = (memcb_pass_t *)calloc(1, sizeof(memcb_pass_t));
    pass->handler = handler;
    pass->access = event->mem_event.out_access;
    if (event->mem_event.out_access & VMI_MEMACCESS_W) {
        //pass->traps = tm_int3traps_at_gfn(handler->trap_manager, event->mem_event.gfn);
        if (pass->traps)
            pass->remapped = tm_find_remapped(handler->trap_manager,event->mem_event.gfn);
    }

    event->slat_id = ORIGIN_IDX;
    handler->step_event[event->vcpu_id]->callback = _post_mem_cb;
    handler->step_event[event->vcpu_id]->data = pass;
    return VMI_EVENT_RESPONSE_TOGGLE_SINGLESTEP
            | VMI_EVENT_RESPONSE_VMM_PAGETABLE_ID;
}

static event_response_t _post_mem_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    memcb_pass_t *pass = event->data;
    vmhdlr_t *handler = pass->handler;
    handler->regs[event->vcpu_id] = event->x86_regs;

    //We need to copy the newly written page to the remapped gfn and reapply all traps
    if (pass->traps) {
        writelog(handler->logid, LV_DEBUG, "Re-copying remapped gfn");
        uint8_t backup[VMI_PS_4KB] = {0};
        if (VMI_FAILURE == vmi_read_pa(handler->vmi, pass->remapped->o<<12, &backup, VMI_PS_4KB)) {
            writelog(handler->logid, LV_ERROR, "Critical error in re-copying remapped gfn\n");
            handler->interrupted = -1;
            return 0;
        }
        if (VMI_FAILURE == vmi_write_pa(handler->vmi, pass->remapped->r<<12, &backup, VMI_PS_4KB)) {
            writelog(handler->logid, LV_ERROR, "Critical error in re-copying remapped gfn");
            handler->interrupted = -1;
            return 0;
        }
        GSList *loop = pass->traps;
        while (loop) {
            uint64_t *pa = loop->data;
            uint8_t test = 0;
            if (VMI_FAILURE == vmi_read_8_pa(handler->vmi, *pa, &test)) {
                writelog(handler->logid, LV_ERROR, "Critical error in re-copying remapped gfn");
                handler->interrupted = -1;
                return 0;
            }
            if (test == INT3_CHAR) {
                tm_set_doubletrap(handler->trap_manager, *pa, 1);
            }
            else {
                tm_set_doubletrap(handler->trap_manager, *pa, 0);
                if (VMI_FAILURE == vmi_write_8_pa(handler->vmi, (pass->remapped->r<<12) + (*pa & VMI_BIT_MASK(0,11)), &INT3_CHAR)) {
                    writelog(handler->logid, LV_ERROR, "Critical error in re-copying remapped gfn");
                    handler->interrupted = -1;
                    return 0;
                }
            }
            loop = loop->next;
        }
    }

    free(pass);
    event->slat_id = handler->altp2m_idx;
    handler->step_event[event->vcpu_id]->callback = _singlestep_cb;
    handler->step_event[event->vcpu_id]->data = handler;
    return VMI_EVENT_RESPONSE_TOGGLE_SINGLESTEP |
            VMI_EVENT_RESPONSE_VMM_PAGETABLE_ID;
}

static event_response_t _singlestep_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    vmhdlr_t *handler = event->data;
    event->slat_id = handler->altp2m_idx;
    return VMI_EVENT_RESPONSE_TOGGLE_SINGLESTEP |
            VMI_EVENT_RESPONSE_VMM_PAGETABLE_ID;
}

hfm_status_t _inject_trap(vmhdlr_t *handler, trap_t *trap)
{
    hfm_status_t status = FAIL;
    addr_t pa = trap->pa;
    addr_t frame = pa >> PAGE_OFFSET_BITS;
    int doubletrap = -1;

    int trap_exist = tm_trap_exist(handler->trap_manager, pa);
    /*
     * Check if there is a trap added to this pa before ?
     * If not, this is the first trap added to this address, so we need to do three things :
     * 1) Create the shadow page for this page (if it's not created yet)
     * 2) Set the memtrap for protecting remapped page
     * 3) Rewrite the instruction at pa with INT3
     */
    if (!trap_exist) {
        //Create shadow page
        remapped_t *remapped = tm_find_remapped(handler->trap_manager, frame);
        if (!remapped) {
            remapped = (remapped_t *)calloc(1, sizeof(remapped_t));
            remapped->o = frame;
            remapped->r = _create_shadow_page(handler, frame);
            if (remapped->r)
                tm_add_remapped(handler->trap_manager, remapped);
        }

        //Callback invoked on a R/W of a monitored page (likely Windows kernel patch protection). Switch the VCPU's SLAT to its original, step once, switch SLAT back
        memtrap_t *memw = tm_find_memtrap(handler->trap_manager, frame);
        if (!memw) {
            //Create new wrapper for memaccess at this page
            memw = (memtrap_t *)calloc(1, sizeof(memtrap_t));
            tm_add_memtrap(handler->trap_manager, g_memdup(&frame, sizeof(uint64_t)), memw);
        }
        vmi_set_mem_event(handler->vmi, frame, VMI_MEMACCESS_RW, handler->altp2m_idx);

        //Set INT3
        addr_t rpa = (remapped->r << PAGE_OFFSET_BITS) + pa % PAGESIZE;
        uint8_t test;
        if (VMI_FAILURE == vmi_read_8_pa(handler->vmi, pa, &test)) {
            writelog(handler->logid, LV_ERROR, "Failed to read 0%lx", pa);
            goto done;
        }
        if (test == INT3_CHAR) {
            doubletrap = 1;
        }
        else {
            doubletrap = 0;
            if (VMI_SUCCESS != vmi_write_8_pa(handler->vmi, rpa, &INT3_CHAR)) {
                writelog(handler->logid, LV_DEBUG, "Failed to write interrupt to shadow page");
                goto done;
            }
        }
    }
    //Insert new trap to trap manager
    tm_add_int3trap(handler->trap_manager, trap);
    if (doubletrap != -1)
        tm_set_doubletrap(handler->trap_manager, pa, doubletrap);
    status = SUCCESS;
done:
    return status;
}

static hfm_status_t _init_vmi(vmhdlr_t *handler)
{
    writelog(handler->logid, LV_INFO, "Init VMI on domain %s", handler->name);
    if (VMI_FAILURE == vmi_init(&handler->vmi, VMI_XEN, handler->name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, NULL, NULL)) {
        writelog(handler->logid, LV_ERROR, "Failed to init LibVMI on domain %s", handler->name);
        goto error1;
    }
    char rekall_profile[STR_BUFF];
    sprintf(rekall_profile, "%s/%s.json", config_get_str(config, "rekall-base"), handler->name);
    GHashTable *vmicfg = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(vmicfg, "rekall_profile", rekall_profile);
    g_hash_table_insert(vmicfg, "os_type", "Windows");
    uint64_t flags = VMI_PM_INITFLAG_TRANSITION_PAGES;
    if (VMI_PM_UNKNOWN == vmi_init_paging(handler->vmi, flags)) {
        g_hash_table_destroy(vmicfg);
        writelog(handler->logid, LV_ERROR, "Failed to init LibVMI paging on domain %s", handler->name);
        goto error2;
    }
    os_t os = vmi_init_os(handler->vmi, VMI_CONFIG_GHASHTABLE, vmicfg, NULL);
    if (os != VMI_OS_WINDOWS) {
        g_hash_table_destroy(vmicfg);
        writelog(handler->logid, LV_ERROR, "Failed to init LibVMI library on domain %s", handler->name);
        goto error2;
    }
    g_hash_table_destroy(vmicfg);

    handler->pm = vmi_get_page_mode(handler->vmi, 0);
    handler->vcpus = vmi_get_num_vcpus(handler->vmi);
    handler->memsize = handler->init_memsize = vmi_get_memsize(handler->vmi);
    handler->winver = vmi_get_winver(handler->vmi);

    //Get domid info
    libxl_name_to_domid(handler->xen->xl_ctx, handler->name, &handler->domid);
    if (!handler->domid || handler->domid == ~0U) {
        writelog(handler->logid, LV_ERROR, "Domain is not running, failed to get domID from name %s!", handler->name);
        goto error2;
    }
    /* Register events */
    int i;
    for (i = 0; i < handler->vcpus && i < 16; i++) {
        handler->step_event[i] = g_malloc0(sizeof(vmi_event_t));
        SETUP_SINGLESTEP_EVENT(handler->step_event[i], 1u << i, _singlestep_cb, 0);
        handler->step_event[i]->data = handler;
        if (VMI_FAILURE == vmi_register_event(handler->vmi, handler->step_event[i])) {
            writelog(handler->logid, LV_ERROR, "Failed to register singlestep for vCPU on %s", handler->name);
            goto error2;
        }
    }
    //Read ObHeaderCookie
    handler->cookie = win_ob_header_cookie(handler);
    //Get all drive devices
    handler->drives = win_list_drives(handler);

    SETUP_INTERRUPT_EVENT(&handler->interrupt_event, 0, _int3_cb);
    handler->interrupt_event.data = handler;
    if (VMI_FAILURE == vmi_register_event(handler->vmi, &handler->interrupt_event)) {
        writelog(handler->logid, LV_ERROR, "Failed to register interrupt event on %s", handler->name);
    }
    SETUP_MEM_EVENT(&handler->mem_event, ~0ULL, VMI_MEMACCESS_RWX, _pre_mem_cb, 1);
    handler->mem_event.data = handler;
    if (VMI_FAILURE == vmi_register_event(handler->vmi, &handler->mem_event)) {
        writelog(handler->logid, LV_ERROR, "Failed to register generic mem event on %s", handler->name);
        goto error2;
    }

    g_mutex_init(&handler->vmi_lock);

    return SUCCESS;
error2:
    vmi_destroy(handler->vmi);
error1:
    return FAIL;
}

static void _close_vmi(vmhdlr_t *handler)
{
    vmi_destroy(handler->vmi);
}

static hfm_status_t _setup_altp2m(vmhdlr_t *handler) {
    status_t status = vmi_slat_set_domain_state(handler->vmi, 1);
    if (status != VMI_SUCCESS) {
        writelog(handler->logid, LV_ERROR, "Failed to enable altp2m on domain %s", handler->name);
        goto error;
    }

    status = vmi_slat_create(handler->vmi, &handler->altp2m_idx);
    if (status != VMI_SUCCESS) {
        writelog(handler->logid, LV_ERROR, "Failed to create altp2m view idx on domain %s", handler->name);
        goto error;
    }

    status = vmi_slat_switch(handler->vmi, handler->altp2m_idx);
    if (status != VMI_SUCCESS) {
        writelog(handler->logid, LV_ERROR, "Failed to switch to altp2m idx view on domain %s", handler->name);
        goto error;
    }
    return SUCCESS;
error:
    return FAIL;
}

static void _reset_altp2m(vmhdlr_t *handler)
{
    vmi_slat_switch(handler->vmi, ORIGIN_IDX);
    vmi_slat_destroy(handler->vmi, handler->altp2m_idx);
}

static void _destroy_traps(vmhdlr_t *handler)
{
    //Reset the memaccess
    GList *memtraps = tm_all_memtraps(handler->trap_manager);
    while (memtraps) {
        uint64_t *gfn = memtraps->data;
        vmi_set_mem_event(handler->vmi, *gfn, VMI_MEMACCESS_N, handler->altp2m_idx);
        memtraps = memtraps->next;
    }

    //Reset remapped frame
    GSList *remappeds = tm_all_remappeds(handler->trap_manager);
    GSList *loop = remappeds;
    while (loop) {
        remapped_t *remapped = loop->data;
        vmi_slat_change_gfn(handler->vmi, handler->altp2m_idx, remapped->o, ~0);
        xen_free_shadow_frame(handler->xen, &remapped->r);
        loop = loop->next;
    }

    //Destroy trap manager
    tm_destroy(handler->trap_manager);
}

static uint64_t _create_shadow_page(vmhdlr_t *handler, uint64_t frame)
{
    /* Setup and activate shadow view */
    uint64_t proposed_memsize = handler->memsize + PAGESIZE;
    uint64_t remapped = xen_alloc_shadow_frame(handler->xen, proposed_memsize);
    if (remapped == 0) {
        writelog(handler->logid, LV_DEBUG, "Extend memory failed for shadow page");
        goto error;
    }
    writelog(handler->logid, LV_DEBUG, "Shadow page created at remapped frame %lx", remapped);
    handler->memsize = proposed_memsize;    //Update current memsize after extend

    /* Change altp2m_idx view to map to new remapped address */
    if (VMI_SUCCESS != vmi_slat_change_gfn(handler->vmi, handler->altp2m_idx, frame, remapped)) {
        writelog(handler->logid, LV_DEBUG, "Failed to update altp2m_idx view to new remapped address");
        goto error;
    }

    /* Copy original page to remapped page */
    uint8_t buff[PAGESIZE] = {0};
    size_t ret = vmi_read_pa(handler->vmi, frame << PAGE_OFFSET_BITS, buff, PAGESIZE);
    if (PAGESIZE != ret) {
        writelog(handler->logid, LV_DEBUG, "Failed to read in syscall page");
        goto error;
    }

    ret = vmi_write_pa(handler->vmi, remapped << PAGE_OFFSET_BITS, buff, PAGESIZE);
    if (PAGESIZE != ret) {
        writelog(handler->logid, LV_DEBUG, "Failed to write to remapped page");
        goto error;
    }

    return remapped;
error:
    return 0;
}

void _remove_int3(vmhdlr_t *handler, addr_t pa)
{
    addr_t gfn = pa >>  PAGE_OFFSET_BITS;
    remapped_t *remapped = tm_find_remapped(handler->trap_manager, gfn);
    uint8_t backup;
    if (VMI_FAILURE == vmi_read_8_pa(handler->vmi, pa, &backup)) {
        writelog(handler->logid, LV_ERROR, "Critical error in removing int3");
        handler->interrupted = -1;
    }
    if (VMI_FAILURE == vmi_write_8_pa(handler->vmi, (remapped->r << PAGE_OFFSET_BITS) + (pa & VMI_BIT_MASK(0,11)), &backup)) {
        writelog(handler->logid, LV_ERROR, "Critical error in removing int3");
        handler->interrupted = -1;
    }
}

vmi_instance_t hfm_lock_and_get_vmi(vmhdlr_t *handler)
{
    g_mutex_lock(&handler->vmi_lock);
    return handler->vmi;
}

void hfm_release_vmi(vmhdlr_t *handler)
{
    g_mutex_unlock(&handler->vmi_lock);
}

int hfm_restart_vmi(void *data)
{
    vmhdlr_t *hdlr = (vmhdlr_t *)data;

    /* Restart xen interface */
    xen_free_interface(hdlr->xen);
    if ((hdlr->xen = xen_init_interface(hdlr->name)) == NULL) {
        writelog(hdlr->logid, LV_ERROR, "Failed to init XEN after machine restart");
        return -1;
    }
    printf("Reinit XEN\n");

    /* Init vmi again */
    if (VMI_FAILURE == vmi_init(&hdlr->vmi, VMI_XEN, hdlr->name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, NULL, NULL)) {
        writelog(hdlr->logid, LV_ERROR, "Failed to init LibVMI after machine restart");
        return -1;
    }
    char rekall_profile[STR_BUFF];
    sprintf(rekall_profile, "%s/%s.json", config_get_str(config, "rekall-base"), hdlr->name);
    GHashTable *vmicfg = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(vmicfg, "rekall_profile", rekall_profile);
    g_hash_table_insert(vmicfg, "os_type", "Windows");
    uint64_t flags = VMI_PM_INITFLAG_TRANSITION_PAGES;
    if (VMI_PM_UNKNOWN == vmi_init_paging(hdlr->vmi, flags)) {
        g_hash_table_destroy(vmicfg);
        writelog(hdlr->logid, LV_ERROR, "Failed to init LibVMI paging after machine restart");
        return -1;
    }
    os_t os = vmi_init_os(hdlr->vmi, VMI_CONFIG_GHASHTABLE, vmicfg, NULL);
    if (os != VMI_OS_WINDOWS) {
        g_hash_table_destroy(vmicfg);
        writelog(hdlr->logid, LV_ERROR, "Failed to init LibVMI library after machine restart");
        return -1;
    }
    g_hash_table_destroy(vmicfg);
    printf("Reinit LibVMI\n");

    /* Register events again */
    int i;
    for (i = 0; i < hdlr->vcpus && i < 16; i++) {
        SETUP_SINGLESTEP_EVENT(hdlr->step_event[i], 1u << i, _singlestep_cb, 0);
        if (VMI_FAILURE == vmi_register_event(hdlr->vmi, hdlr->step_event[i])) {
            writelog(hdlr->logid, LV_ERROR, "Failed to register singlestep for vCPU on %s", hdlr->name);
            return -1;
        }
    }
    SETUP_INTERRUPT_EVENT(&hdlr->interrupt_event, 0, _int3_cb);
    if (VMI_FAILURE == vmi_register_event(hdlr->vmi, &hdlr->interrupt_event)) {
        writelog(hdlr->logid, LV_ERROR, "Failed to register interrupt event on %s", hdlr->name);
        return -1;
    }
    SETUP_MEM_EVENT(&hdlr->mem_event, ~0ULL, VMI_MEMACCESS_RWX, _pre_mem_cb, 1);
    if (VMI_FAILURE == vmi_register_event(hdlr->vmi, &hdlr->mem_event)) {
        writelog(hdlr->logid, LV_ERROR, "Failed to register generic mem event on %s", hdlr->name);
        return -1;
    }
    printf("Finish register events\n");
    /* Create altp2m view */
    if (SUCCESS != _setup_altp2m(hdlr)) {
        writelog(hdlr->logid, LV_ERROR, "Failed to re-init altp2m view");
        return -1;
    }
    printf("Finish setup_altp2m\n");
    /* Init trap manager */
    tm_destroy(hdlr->trap_manager);
    hdlr->trap_manager = tm_init();
    if (hdlr->trap_manager == NULL) {
        writelog(hdlr->logid, LV_ERROR, "Failed to re-init trap manager");
        return -1;
    }
    printf("Finish init trap manager\n");
    //* Set policies */
    hdlr->file_created_init = 0;
    hdlr->file_deleted_init = 0;
    hdlr->file_modified_init = 0;
    hdlr->attr_changed_init = 0;
    hdlr->access_changed_init = 0;
    hfm_set_policies(hdlr, hdlr->policies);
    printf("Finish set policies\n");

    return 0;
}

void hfm_set_policies(vmhdlr_t *handler, GHashTable *policies)
{
    if (policies == NULL) return;
    GList *list = g_hash_table_get_values(policies);
    GList *it = NULL;
    for (it = list; it; it = it->next) {
        policy_t *pol = (policy_t *)(it->data);
        mon_add_policy(handler, pol);
    }
}

