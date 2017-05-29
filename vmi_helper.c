#include <libvmi/libvmi.h>
#include <libvmi/events.h>

#include "vmi_helper.h"

static event_response_t _int3_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _pre_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _post_mem_cb(vmi_instance_t vmi, vmi_event_t *event);
static event_response_t _singlestep_cb(vmi_instance_t vmi, vmi_event_t *event);
static bool _add_trap(vmhdlr_t *handler, trap_t *trap);
static bool _remove_trap(vmhdlr_t *handler, trap_t *trap);

hfm_status_t vh_init(vmhdlr_t *handler)
{
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
