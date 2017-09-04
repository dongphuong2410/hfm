#include "file_created.h"
#include "file_deleted.h"
#include "file_modified.h"
#include "attr_changed.h"
#include "libmon.h"
#include "private.h"
#include "config.h"
#include "log.h"
#include "constants.h"
#include "file_filter.h"


config_t *config;

int mon_init(void)
{
    return 0;
}

hfm_status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    switch (policy->type) {
        case MON_CREATE:
            file_created_add_policy(hdlr, policy);
            break;
        case MON_DELETE:
            file_deleted_add_policy(hdlr, policy);
            break;
        case MON_MODIFY_CONTENT:
            file_modified_add_policy(hdlr, policy);
            break;
        case MON_CHANGE_ATTR:
            attr_changed_add_policy(hdlr, policy);
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

void mon_close()
{
    file_created_close();
}
