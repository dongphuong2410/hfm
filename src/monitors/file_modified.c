#include <stdio.h>
#include <stdlib.h>
#include "file_filter.h"
#include "monitor.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"
#include "config.h"

extern config_t *config;

typedef struct params_t {
    char filename[STR_BUFF];
    addr_t file_object;
    addr_t io_status_addr;
    int policy_id;
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
        filter = filter_init();
    }
    if (!hdlr->file_modified_init) {
        hfm_monitor_syscall(hdlr, "NtWriteFile", writefile_cb, writefile_ret_cb);
        hfm_monitor_syscall(hdlr, "ZwWriteFile", writefile_cb, writefile_ret_cb);
        hdlr->file_modified_init = 1;
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
    addr_t io_status_addr = 0;

    if (handler->pm == VMI_PM_IA32E) {
        handle = context->regs->rcx;
        io_status_addr = hfm_read_addr(context, context->regs->rsp + 5 * sizeof(addr_t));
        buffer_length = hfm_read_32(context, context->regs->rsp + 7 * sizeof(addr_t));
    }
    else {
        handle = hfm_read_32(context, context->regs->rsp + 1 * sizeof(uint32_t));
        io_status_addr = hfm_read_32(context, context->regs->rsp + 5 * sizeof(uint32_t));
        buffer_length = hfm_read_32(context, context->regs->rsp + 7 * sizeof(uint32_t));
    }

    char filename[STR_BUFF] = "";
    addr_t file_object = hfm_fileobj_from_handle(vmi, context, handle);
    int len = hfm_read_filename_from_object(vmi, context, file_object, filename);
    int policy_id;
    if (filename[0] && (policy_id = filter_match(filter, filename)) >= 0) {
        params = (params_t *)calloc(1, sizeof(params_t));
        strncpy(params->filename, filename, len);
        params->io_status_addr = io_status_addr;
        params->policy_id = policy_id;
        params->file_object = file_object;
    }
done:
    hfm_release_vmi(handler);
    return params;
}

static void *writefile_ret_cb(vmhdlr_t *handler, context_t *context)
{
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    params_t *params = (params_t *)context->trap->extra;
    int status = context->regs->rax;
    uint64_t information = hfm_read_64(context, params->io_status_addr + context->hdlr->offsets[IO_STATUS_BLOCK__INFORMATION]);
    if (NT_SUCCESS(status)) {
        output_info_t output;
        output.pid = hfm_get_process_pid(vmi, context);
        struct timeval now;
        gettimeofday(&now, NULL);
        output.time_sec = now.tv_sec;
        output.time_usec = now.tv_usec;
        output.vmid = handler->domid;
        output.action = MON_MODIFY_CONTENT;
        output.policy_id = params->policy_id;
        strncpy(output.filepath, params->filename, PATH_MAX_LEN);
        output.extpath[0] = '\0';
        if (config_get_int(config, "file-extract")) {
            policy_t *policy = g_hash_table_lookup(handler->policies, &params->policy_id);
            if (policy->options & POLICY_OPTIONS_EXTRACT) {
                char *dir = config_get_str(config, "extract_base");
                int extracted = hfm_extract_file(vmi, context, params->file_object, output.extpath);
                if (extracted)
                    sprintf(output.extpath, "%s%s/%u_%u.file", dir ? dir : "", context->hdlr->name,  output.time_sec, output.time_usec);
            }
        }
        out_write(handler->out, &output);
    }
    free(params);
done:
    hfm_release_vmi(handler);
    return NULL;
    return NULL;
}

void file_modified_close(void)
{
    if (filter) {
        filter_close(filter);
        filter = NULL;
    }
}
