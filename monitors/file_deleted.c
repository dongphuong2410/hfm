#include "file_deleted.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"
#include "config.h"


extern config_t *config;
static filter_t *filter = NULL;

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is called
  */
static void *setinformation_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is returned
  */
static void *setinformation_ret_cb(vmhdlr_t *handler, context_t *context);

hfm_status_t file_deleted_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    if (!filter) {
        //Init plugin
        filter = filter_init();
        hfm_monitor_syscall(hdlr, "NtSetInformationFile", setinformation_cb, setinformation_ret_cb);
        hfm_monitor_syscall(hdlr, "ZwSetInformationFile", setinformation_cb, setinformation_ret_cb);
    }
    filter_add(filter, policy->path, policy->id);
    return SUCCESS;
}
/**
  * - Read FileInformationClass, check if class is FILE_DISPOSITION_INFORMATION
  * - Read FILE_DISPOSITION_INFORMATION struct for DeleteFile field
  * - Read FileHandle, searching for FILE_OBJECT, read FileName from FILE_OBJECT
  */
static void *setinformation_cb(vmhdlr_t *handler, context_t *context)
{
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    addr_t fileinfo_addr = 0;
    uint32_t fileinfo_class = 0;
    reg_t handle = 0;

    //Read FileInformationClass and address of FileInformation
    if (handler->pm == VMI_PM_IA32E) {
        handle = context->regs->rcx;
        fileinfo_addr = context->regs->r8;
        fileinfo_class = hfm_read_32(vmi, context, context->regs->rsp + 5 * sizeof(addr_t));
    }
    else {
        handle = hfm_read_32(vmi, context, context->regs->rsp + 1 * sizeof(uint32_t));
        fileinfo_addr = hfm_read_32(vmi, context, context->regs->rsp + 3 * sizeof(uint32_t));
        fileinfo_class = hfm_read_32(vmi, context, context->regs->rsp + 5 * sizeof(uint32_t));
    }
    if (FILE_DISPOSITION_INFORMATION == fileinfo_class) {
        char filename[STR_BUFF] = "";
        addr_t file_object = hfm_fileobj_from_handle(vmi, context, handle);
        hfm_read_filename_from_object(vmi, context, file_object, filename);
        int policy_id = filter_match(filter, filename);
        if (policy_id >= 0) {
            uint8_t delete = hfm_read_8(vmi, context, fileinfo_addr + context->hdlr->offsets[FILE_DISPOSITION_INFORMATION__DELETE_FILE]);
            if (delete) {
                output_info_t output;
                output.pid = hfm_get_process_pid(vmi, context);
                struct timeval now;
                gettimeofday(&now, NULL);
                output.time_sec = now.tv_sec;
                output.time_usec = now.tv_usec;
                output.vmid = handler->domid;
                output.action = MON_DELETE;
                output.policy_id = policy_id;
                strncpy(output.filepath, filename, PATH_MAX_LEN);
                char *dir = config_get_str(config, "extract_base");
                sprintf(output.extpath, "%s%s/%u_%u.file", dir ? dir : "", context->hdlr->name, output.time_sec, output.time_usec);
                int extracted = hfm_extract_file(vmi, context, file_object, output.extpath);
                if (extracted == 0) {
                    output.extpath[0] = '\0';
                }
                out_write(handler->out, &output);
            }
        }
    }
done:
    hfm_release_vmi(handler);
    return NULL;
}

//Not used yet
static void *setinformation_ret_cb(vmhdlr_t *handler, context_t *context)
{
    return NULL;
}
