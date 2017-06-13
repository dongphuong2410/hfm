#include "libmon.h"
#include "libhfm.h"


void mon_init(monitor_t type)
{
}

hfm_status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtCreateFile");
    hfm_monitor_syscall(hdlr, "NtOpenFile");
    return SUCCESS;
}
