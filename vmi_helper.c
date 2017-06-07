#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <glib.h>

#include "vmi_helper.h"
#include "log.h"

static event_response_t _int3_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _pre_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _post_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _singlestep_cb(vmi_instance_t vmi, vmi_event_t *event);
static bool _add_trap(vmhdlr_t *handler, trap_t *trap);
static bool _remove_trap(vmhdlr_t *handler, trap_t *trap);

static hfm_status_t _init_vmi(vmhdlr_t *handler);
static hfm_status_t _register_events(vmhdlr_t *handler);

hfm_status_t vh_init(vmhdlr_t *handler)
{
    /* Init LibVMI */
    if (SUCCESS != _init_vmi(handler)) {
        goto error;
    }
    /* Register events */
    if (SUCCESS != _register_events(handler)) {
        goto error;
    }
    /* Create altp2m view */
    int rc = xc_altp2m_set_domain_state(handler->xen->xc, handler->domID, 1);
    if (rc < 0) {
        writelog(LV_ERROR, "Failed to enable altp2m on domain %s", handler->name);
    }

    return SUCCESS;
error:
    return FAIL;
}

hfm_status_t vh_run(vmhdlr_t *handler)
{
    return FAIL;
}

hfm_status_t vh_monitor_syscall(vmhdlr_t *handler, const char *name, void *pre_cb, void *post_cb)
{
    return FAIL;
}

void vh_close(vmhdlr_t *handler)
{
    writelog(LV_INFO, "Close LibVMI on domain %s", handler->name);
    if (handler->vmi)
        vmi_destroy(handler->vmi);
}

static event_response_t _int3_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    return 0;
}

static event_response_t _pre_mem_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    return 0;
}

static event_response_t _post_mem_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    return 0;
}

static event_response_t _singlestep_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    return 0;
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
    GHashTable *vmicfg = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(vmicfg, "rekall_profile", handler->rekall);
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
    return SUCCESS;
error:
    return FAIL;
}

static hfm_status_t _register_events(vmhdlr_t *handler)
{
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
