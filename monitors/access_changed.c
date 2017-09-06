#include "monitor.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"

#define OWNER_SECURITY_INFORMATION 0x00000001
#define GROUP_SECURITY_INFORMATION 0x00000002
#define DACL_SECURITY_INFORMATION  0x00000004
#define SACL_SECURITY_INFORMATION  0x00000008
#define LABEL_SECURITY_INFORMATION 0x00000010

typedef struct params_t {
    char filename[STR_BUFF];
    uint32_t security_info;
    uint32_t policy_id;
} params_t;

static filter_t *filter = NULL;

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is called
  */
static void *setsecurity_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is returned
  */
static void *setsecurity_ret_cb(vmhdlr_t *handler, context_t *context);

/**
  * Generate text from security info code
  */
static inline void _set_security_info_text(uint32_t info, char *buff);

hfm_status_t access_changed_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    if (!filter) {
        //Init plugin
        filter = filter_init();
        hfm_monitor_syscall(hdlr, "NtSetSecurityObject", setsecurity_cb, setsecurity_ret_cb);
        hfm_monitor_syscall(hdlr, "ZwSetSecurityObject", setsecurity_cb, setsecurity_ret_cb);
    }
    filter_add(filter, policy->path, policy->id);
    return SUCCESS;
}

static void *setsecurity_cb(vmhdlr_t *handler, context_t *context)
{
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    reg_t handle = 0;
    uint32_t security_info = 0;
    addr_t security_desc_addr = 0;
    params_t *params = NULL;

    //Read handle address, security info, security descriptor address
    if (handler->pm == VMI_PM_IA32E) {
        handle = context->regs->rcx;
        security_info = context->regs->rdx;
        security_desc_addr = context->regs->r8;
    }
    else {
        handle = hfm_read_32(vmi, context, context->regs->rsp + 1 * sizeof(uint32_t));
        security_info = hfm_read_32(vmi, context, context->regs->rsp + 2 * sizeof(uint32_t));
        security_desc_addr = hfm_read_32(vmi, context, context->regs->rsp + 3 * sizeof(uint32_t));
    }
    char filename[STR_BUFF] = "";
    addr_t file_object = hfm_fileobj_from_handle(vmi, context, handle);
    hfm_read_filename_from_object(vmi, context, file_object, filename);
    int policy_id = filter_match(filter, filename);
    if (policy_id  > 0) {
        params = (params_t *)calloc(1, sizeof(params_t));
        strncpy(params->filename, filename, STR_BUFF);
        params->policy_id = policy_id;
        params->security_info = security_info;
    }
done:
    hfm_release_vmi(handler);
    return params;
}

static void *setsecurity_ret_cb(vmhdlr_t *handler, context_t *context)
{
    params_t *params = (params_t *)context->trap->extra;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    int ret_status = context->regs->rax;
    if (STATUS_SUCCESS == ret_status) {
        output_info_t output;
        output.pid = hfm_get_process_pid(vmi, context);
        struct timeval now;
        gettimeofday(&now, NULL);
        output.time_sec = now.tv_sec;
        output.time_usec = now.tv_usec;
        output.vmid = handler->domid;
        output.action = MON_CHANGE_ACCESS;
        output.policy_id = params->policy_id;
        strncpy(output.filepath, params->filename, PATH_MAX_LEN);
        _set_security_info_text(params->security_info, output.data);
        output.extpath[0] = '\0';
        out_write(handler->out, &output);
    }
    free(params);
done:
    hfm_release_vmi(handler);
    return NULL;
}

void access_changed_close(void)
{
    if (filter) filter_close(filter);
}

static inline void _set_security_info_text(uint32_t info, char *buff)
{
    int pos = 0;
    if (info & OWNER_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "Owner Changed;");
    }
    if (info & GROUP_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "Group Changed;");
    }
    if (info & DACL_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "DACL Changed;");
    }
    if (info & SACL_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "SACL Changed;");
    }
    if (info & LABEL_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "Label Changed;");
    }
}
