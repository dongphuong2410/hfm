#include <stdio.h>
#include <stdlib.h>
#include "file_modified.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"

typedef struct params_t {
    char filename[STR_BUFF];
} params_t;

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
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    params_t *params = NULL;
    reg_t handle = 0;
    uint32_t buffer_length = 0;

    if (handler->pm == VMI_PM_IA32E) {
        handle = context->regs->rcx;
        buffer_length = hfm_read_32(vmi, context, context->regs->rsp + 7 * sizeof(addr_t));
    }
    else {
        handle = hfm_read_32(vmi, context, context->regs->rsp + 1 * sizeof(uint32_t));
        buffer_length = hfm_read_32(vmi, context, context->regs->rsp + 7 * sizeof(uint32_t));
    }

    char filename[STR_BUFF] = "";
    int len = hfm_read_filename_from_handler(vmi, context, handle, filename);
    if (filename[0] && filter_match(filter, filename) >= 0) {
        params = (params_t *)calloc(1, sizeof(params_t));
        strncpy(params->filename, filename, len);
    }
done:
    hfm_release_vmi(handler);
    return params;
}

static void *writefile_ret_cb(vmhdlr_t *handler, context_t *context)
{
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    params_t *params = (params_t *)context->trap->extra;
    printf("[MODIFY] %s\n", params->filename);
    free(params);
done:
    hfm_release_vmi(handler);
    return NULL;
    return NULL;
}
