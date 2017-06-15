#include "libmon.h"
#include "hfm.h"
#include "private.h"


void mon_init(monitor_t type)
{
}

event_response_t cb_create(vmhdlr_t *handler, trap_data_t *data);
event_response_t cb_open(vmhdlr_t *handler, trap_data_t *data);

hfm_status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtCreateFile", cb_create);
    hfm_monitor_syscall(hdlr, "NtOpenFile", cb_open);
    return SUCCESS;
}

event_response_t cb_create(vmhdlr_t *handler, trap_data_t *data)
{
    printf("Callback NtCreateFile\n");
}

event_response_t cb_open(vmhdlr_t *handler, trap_data_t *data)
{
    printf("Callback NtOpenFile\n");
}
