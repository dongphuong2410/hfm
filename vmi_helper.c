#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <libvmi/slat.h>
#include <glib.h>

#include "vmi_helper.h"
#include "xen_helper.h"
#include "config.h"
#include "log.h"

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

static event_response_t _int3_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _pre_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _post_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _singlestep_cb(vmi_instance_t vmi, vmi_event_t *event);
static bool _add_trap(vmhdlr_t *handler, trap_t *trap);
static bool _remove_trap(vmhdlr_t *handler, trap_t *trap);

static hfm_status_t _init_vmi(vmhdlr_t *handler);
static void _close_vmi(vmhdlr_t *handler);
static hfm_status_t _setup_altp2m(vmhdlr_t *handler);
static void _reset_altp2m(vmhdlr_t *handler);

extern config_t *config;

hfm_status_t vh_init(vmhdlr_t *handler)
{
    /* Init LibVMI */
    if (SUCCESS != _init_vmi(handler)) {
        goto error;
    }
    /* Init xen interface*/
    if ((handler->xen = xen_init_interface(handler->name)) == NULL) {
        writelog(LV_ERROR, "Failed to init XEN on domain %s", handler->name);
        xen_free_interface(handler->xen);
        goto error;
    }
    /* Create altp2m view */
    if (SUCCESS != _setup_altp2m(handler)) {
        goto error;
    }

    return SUCCESS;
error:
    return FAIL;
}

void vh_close(vmhdlr_t *handler)
{
    writelog(LV_INFO, "Close LibVMI on domain %s", handler->name);
    vmi_pause_vm(handler->vmi);

    /* Reset altp2m view */
    _reset_altp2m(handler);

    /* Close xen interface */
    xen_free_interface(handler->xen);

    vmi_resume_vm(handler->vmi);

    /* Close LibVMI */
    _close_vmi(handler);
}

void vh_listen(vmhdlr_t *handler)
{
    vmi_events_listen(handler->vmi, 500);
}

hfm_status_t vh_monitor_syscall(vmhdlr_t *handler, const char *name, void *pre_cb, void *post_cb)
{
    return FAIL;
}


static event_response_t _int3_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    event_response_t rsp = 0;
    vmhdlr_t *handler = event->data;
    printf("Breakpoint triggered\n");
    event->slat_id = ORIGIN_IDX;
    event->interrupt_event.reinject = 0;
    handler->step_event[event->vcpu_id]->callback = _singlestep_cb;
    handler->step_event[event->vcpu_id]->data = handler;
    return rsp |
            VMI_EVENT_RESPONSE_TOGGLE_SINGLESTEP |
            VMI_EVENT_RESPONSE_VMM_PAGETABLE_ID;
}

static event_response_t _pre_mem_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    printf("Mem_cb called\n");
    event->slat_id = ORIGIN_IDX;
    return VMI_EVENT_RESPONSE_TOGGLE_SINGLESTEP
            | VMI_EVENT_RESPONSE_VMM_PAGETABLE_ID;
}

static event_response_t _post_mem_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    return 0;
}

static event_response_t _singlestep_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    vmhdlr_t *handler = event->data;
    event->slat_id = handler->altp2m_idx;
    return VMI_EVENT_RESPONSE_TOGGLE_SINGLESTEP |
            VMI_EVENT_RESPONSE_VMM_PAGETABLE_ID;
}

static bool _add_trap(vmhdlr_t *handler, trap_t *trap)
{
    return 0;
}

static bool _remove_trap(vmhdlr_t *handler, trap_t *trap)
{
    return 0;
}

static hfm_status_t _init_vmi(vmhdlr_t *handler)
{
    writelog(LV_INFO, "Init VMI on domain %s", handler->name);
    if (VMI_FAILURE == vmi_init(&handler->vmi, VMI_XEN, handler->name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, NULL, NULL)) {
        writelog(LV_ERROR, "Failed to init LibVMI on domain %s", handler->name);
        goto error;
    }
    char *rekall_profile = config_get_str(config, "rekall_profile");
    GHashTable *vmicfg = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(vmicfg, "rekall_profile", rekall_profile);
    g_hash_table_insert(vmicfg, "os_type", "Windows");
    uint64_t flags = VMI_PM_INITFLAG_TRANSITION_PAGES;
    if (VMI_PM_UNKNOWN == vmi_init_paging(handler->vmi, flags)) {
        writelog(LV_ERROR, "Failed to init LibVMI paging on domain %s", handler->name);
        g_hash_table_destroy(vmicfg);
        goto error;
    }
    os_t os = vmi_init_os(handler->vmi, VMI_CONFIG_GHASHTABLE, vmicfg, NULL);
    g_hash_table_destroy(vmicfg);
    if (os != VMI_OS_WINDOWS) {
        writelog(LV_ERROR, "Failed to init LibVMI library on domain %s", handler->name);
        goto error;
    }
    handler->pm = vmi_get_page_mode(handler->vmi, 0);
    handler->vcpus = vmi_get_num_vcpus(handler->vmi);
    handler->memsize = handler->init_memsize = vmi_get_memsize(handler->vmi);

    /* Register events */
    int i;
    for (i = 0; i < handler->vcpus && i < 16; i++) {
        handler->step_event[i] = g_malloc0(sizeof(vmi_event_t));
        SETUP_SINGLESTEP_EVENT(handler->step_event[i], 1u << i, _singlestep_cb, 0);
        handler->step_event[i]->data = handler;
        if (VMI_FAILURE == vmi_register_event(handler->vmi, handler->step_event[i])) {
            writelog(LV_ERROR, "Failed to register singlestep for vCPU on %s", handler->name);
            goto error;
        }
    }
    SETUP_INTERRUPT_EVENT(&handler->interrupt_event, 0, _int3_cb);
    handler->interrupt_event.data = handler;
    if (VMI_FAILURE == vmi_register_event(handler->vmi, &handler->interrupt_event)) {
        writelog(LV_ERROR, "Failed to register interrupt event on %s", handler->name);
    }
    SETUP_MEM_EVENT(&handler->mem_event, ~0ULL, VMI_MEMACCESS_RWX, _pre_mem_cb, 1);
    handler->mem_event.data = handler;
    if (VMI_FAILURE == vmi_register_event(handler->vmi, &handler->mem_event)) {
        writelog(LV_ERROR, "Failed to register generic mem event on %s", handler->name);
        goto error;
    }
    return SUCCESS;
error:
    return FAIL;
}

static void _close_vmi(vmhdlr_t *handler)
{
    vmi_destroy(handler->vmi);
}

static hfm_status_t _setup_altp2m(vmhdlr_t *handler) {
    status_t status = vmi_slat_set_domain_state(handler->vmi, 1);
    if (status != VMI_SUCCESS) {
        writelog(LV_ERROR, "Failed to enable altp2m on domain %s", handler->name);
        goto error;
    }

    status = vmi_slat_create(handler->vmi, &handler->altp2m_idx);
    if (status != VMI_SUCCESS) {
        writelog(LV_ERROR, "Failed to create altp2m view idx on domain %s", handler->name);
        goto error;
    }

    status = vmi_slat_switch(handler->vmi, handler->altp2m_idx);
    if (status != VMI_SUCCESS) {
        writelog(LV_ERROR, "Failed to switch to altp2m idx view on domain %s", handler->name);
        goto error;
    }
    return SUCCESS;
error:
    return FAIL;
}

static void _reset_altp2m(vmhdlr_t *handler) {
    vmi_slat_switch(handler->vmi, ORIGIN_IDX);
    vmi_slat_destroy(handler->vmi, handler->altp2m_idx);
}
