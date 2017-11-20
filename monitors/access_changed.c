#include "monitor.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "log.h"
#include "constants.h"

#define OWNER_SECURITY_INFORMATION 0x00000001
#define GROUP_SECURITY_INFORMATION 0x00000002
#define DACL_SECURITY_INFORMATION  0x00000004
#define SACL_SECURITY_INFORMATION  0x00000008
#define LABEL_SECURITY_INFORMATION 0x00000010

#define SE_SELF_RELATIVE           0x8000

typedef struct params_t {
    char filename[STR_BUFF];
    uint32_t policy_id;
    char detail[STR_BUFF];
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
static inline void _set_security_info_text(context_t *context, uint32_t info, addr_t sd_addr, char *buff);

/**
  * Extract SID
  * @param[in] context Context
  * @param[in] sid_addr SID address
  * @param[out] sid SID text
  * Reference : https://github.com/libyal/libfwnt/wiki/Security-Descriptor
  */
static void _extract_sid(context_t *context, addr_t sid_addr, char *sid);

hfm_status_t access_changed_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    if (!filter) {
        filter = filter_init();
    }
    if (!hdlr->access_changed_init) {
        hfm_monitor_syscall(hdlr, "NtSetSecurityObject", setsecurity_cb, setsecurity_ret_cb);
        hfm_monitor_syscall(hdlr, "ZwSetSecurityObject", setsecurity_cb, setsecurity_ret_cb);
        hdlr->access_changed_init = 1;
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
        handle = hfm_read_32(context, context->regs->rsp + 1 * sizeof(uint32_t));
        security_info = hfm_read_32(context, context->regs->rsp + 2 * sizeof(uint32_t));
        security_desc_addr = hfm_read_32(context, context->regs->rsp + 3 * sizeof(uint32_t));
    }
    char filename[STR_BUFF] = "";
    addr_t file_object = hfm_fileobj_from_handle(vmi, context, handle);
    hfm_read_filename_from_object(vmi, context, file_object, filename);
    int policy_id = filter_match(filter, filename);
    if (policy_id  > 0) {
        params = (params_t *)calloc(1, sizeof(params_t));
        strncpy(params->filename, filename, STR_BUFF);
        params->policy_id = policy_id;
        _set_security_info_text(context, security_info, security_desc_addr, params->detail);
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
        strncpy(output.data, params->detail, STR_BUFF);
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
    if (filter) {
        filter_close(filter);
        filter = NULL;
    }
}

static inline void _set_security_info_text(context_t *context, uint32_t info, addr_t sd_addr, char *buff)
{
    int pos = 0;
    char detail[STR_BUFF] = "";
    if (info & OWNER_SECURITY_INFORMATION) {
        uint16_t control = 0;
        control = hfm_read_16(context, sd_addr + context->hdlr->offsets[SECURITY_DESCRIPTOR__CONTROL]);
        addr_t owner_addr = 0;
        if (control & SE_SELF_RELATIVE) {
            owner_addr = sd_addr + hfm_read_32(context, sd_addr + context->hdlr->offsets[SECURITY_DESCRIPTOR_RELATIVE__OWNER]);
        }
        else {
            owner_addr = hfm_read_addr(context, sd_addr + context->hdlr->offsets[SECURITY_DESCRIPTOR__OWNER]);
        }
        _extract_sid(context, owner_addr, detail);
        pos += sprintf(buff + pos, "%s : %s", "Owner Changed;", detail);
    }
    if (info & GROUP_SECURITY_INFORMATION) {
        uint16_t control = 0;
        control = hfm_read_16(context, sd_addr + context->hdlr->offsets[SECURITY_DESCRIPTOR__CONTROL]);
        addr_t group_addr = 0;
        if (control & SE_SELF_RELATIVE) {
            group_addr = sd_addr + hfm_read_32(context, sd_addr + context->hdlr->offsets[SECURITY_DESCRIPTOR_RELATIVE__GROUP]);
        }
        else {
            group_addr = hfm_read_addr(context, sd_addr + context->hdlr->offsets[SECURITY_DESCRIPTOR__GROUP]);
        }
        _extract_sid(context, group_addr, detail);
        pos += sprintf(buff + pos, "%s : %s", "Group Changed;", detail);
    }
    if (info & DACL_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "%s", "DACL Changed;");
    }
    if (info & SACL_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "%s", "SACL Changed;");
    }
    if (info & LABEL_SECURITY_INFORMATION) {
        pos += sprintf(buff + pos, "%s", "Label Changed;");
    }
}

static void _extract_sid(context_t *context, addr_t sid_addr, char *sid)
{
    int i;
    int pos = 0;

    //First byte
    uint8_t first_byte = hfm_read_8(context, sid_addr + 0);
    if (first_byte >= 0 && first_byte <= 9)
        pos += sprintf(sid, "S-%c-", first_byte + '0');
    else {
        writelog(LV_DEBUG, "Invalid SID");
        return;
    }
    //Authority part
    uint64_t authority = 0;
    uint8_t bytes[6];
    for (i = 0; i < 6; i++) {
        bytes[i] = hfm_read_8(context, sid_addr + 2 + i);
        authority += (((uint64_t)bytes[i]) << (5 - i)*8);
    }
    pos += sprintf(sid + pos, "%lu-", authority);

    //Sub authorities
    uint8_t sub_auth_no = hfm_read_8(context, sid_addr + 1);
    for (i = 0; i < sub_auth_no; i++) {
        uint32_t sub_auth = hfm_read_32(context, sid_addr + 8 + 4 * i);
        pos += sprintf(sid + pos, "%u-", sub_auth);
    }

    //Finish extract SID
    sid[pos-1] = '\0';  //Remove the last '-' character
}
