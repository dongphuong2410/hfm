#include "libmon.h"
#include "hfm.h"
#include "private.h"
#include "log.h"


void mon_init(monitor_t type)
{
}

event_response_t syscall_cb(vmhdlr_t *handler, trap_data_t *data);
event_response_t sysret_cb(vmhdlr_t *handler, trap_data_t *data);

hfm_status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtCreateFile", syscall_cb, sysret_cb);
    //hfm_monitor_syscall(hdlr, "NtCreateFile", cb_create, NULL);
    //hfm_monitor_syscall(hdlr, "NtOpenFile", cb_open, NULL);
    return SUCCESS;
}

event_response_t syscall_cb(vmhdlr_t *handler, trap_data_t *data)
{
    writelog(LV_DEBUG, "NtCreateFile start");
}

event_response_t sysret_cb(vmhdlr_t *handler, trap_data_t *data)
{
    writelog(LV_DEBUG, "NtCreateFile return");
}
