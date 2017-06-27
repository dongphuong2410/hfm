#include "libmon.h"
#include "hfm.h"
#include "private.h"
#include "log.h"


void mon_init(monitor_t type)
{
}

int syscall_cb(vmhdlr_t *handler, trap_data_t *data);
int sysret_cb(vmhdlr_t *handler, trap_data_t *data);

hfm_status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtCreateFile", syscall_cb, sysret_cb);
    //hfm_monitor_syscall(hdlr, "NtCreateFile", cb_create, NULL);
    //hfm_monitor_syscall(hdlr, "NtOpenFile", cb_open, NULL);
    return SUCCESS;
}

int syscall_cb(vmhdlr_t *handler, trap_data_t *data)
{
    writelog(LV_DEBUG, "NtCreateFile start");
    return 1;
}

int sysret_cb(vmhdlr_t *handler, trap_data_t *data)
{
    writelog(LV_DEBUG, "NtCreateFile return");
    return 0;
}
