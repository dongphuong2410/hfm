#include "file_modified.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"

static filter_t *filter = NULL;

/**
  * Callback when the functions NtWriteFile, ZtWriteFile is called
  */
static void *writefile_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtWriteFile, ZwWriteFile is returned
  */
static void *writefile_ret_cb(vmhdlr_t *handler, context_t *context);

hfm_status_t file_modified_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    if (!filter) {
        //Init plugin
        filter = filter_init();
        hfm_monitor_syscall(hdlr, "NtWriteFile", writefile_cb, writefile_ret_cb);
        hfm_monitor_syscall(hdlr, "ZwWriteFile", writefile_cb, writefile_ret_cb);
    }
    filter_add(filter, policy->path, policy->id);
    return SUCCESS;
}

static void *writefile_cb(vmhdlr_t *handler, context_t *context)
{
    printf("NtWriteFile called\n");
    return NULL;
}

static void *writefile_ret_cb(vmhdlr_t *handler, context_t *context)
{
    return NULL;
}
