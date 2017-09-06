#include "monitor.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"

static filter_t *filter = NULL;

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is called
  */
static void *setsecurity_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is returned
  */
static void *setsecurity_ret_cb(vmhdlr_t *handler, context_t *context);

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
        printf("Set security cb %u\n", security_info);
    }
done:
    hfm_release_vmi(handler);
    return NULL;
}

//Not used yet
static void *setsecurity_ret_cb(vmhdlr_t *handler, context_t *context)
{
    return NULL;
}

void access_changed_close(void)
{
    if (filter) filter_close(filter);
}
