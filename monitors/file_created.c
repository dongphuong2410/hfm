#include "file_created.h"
#include "hfm.h"
#include "log.h"

int syscall_cb(vmhdlr_t *handler, trap_data_t *data);
int sysret_cb(vmhdlr_t *handler, trap_data_t *data);

hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtCreateFile", syscall_cb, sysret_cb);
    return FAIL;
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
