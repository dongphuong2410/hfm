#include "file_created.h"
#include "libmon.h"
#include "private.h"


void mon_init(monitor_t type)
{
}

hfm_status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    switch (policy->type) {
        case MON_CREATE:
            file_created_add_policy(hdlr, policy);
            break;
        case MON_DELETE:
            break;
        case MON_MODIFY_CONTENT:
            break;
        case MON_MODIFY_LOGFILE:
            break;
        case MON_CHANGE_ATTR_READONLY:
            break;
        case MON_CHANGE_ATTR_PERMISSION:
            break;
        case MON_CHANGE_ATTR_OWNERSHIP:
            break;
        case MON_CHANGE_ATTR_HIDDEN:
            break;
        default:
            break;
    }
    return SUCCESS;
}
