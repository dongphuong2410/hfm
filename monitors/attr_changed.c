#include "monitor.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"

#define ATTR_READ_ONLY              0x1
#define ATTR_HIDDEN                 0x2
#define ATTR_SYSTEM                 0x4
#define ATTR_DIRECTORY              0x10
#define ATTR_ARCHIVE                0x20
#define ATTR_NORMAL                 0x80
#define ATTR_TEMPORARY              0x100
#define ATTR_SPARSE_FILE            0x200
#define ATTR_SYMBOLIC_LINK          0x400
#define ATTR_ENCRYPTED              0x4000

static filter_t *filter = NULL;

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is called
  */
static void *setinformation_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is returned
  */
static void *setinformation_ret_cb(vmhdlr_t *handler, context_t *context);

/**
  * Translate Windows file attributes to string
  */
static void _attr_to_str(uint32_t attr, char *buff);

hfm_status_t attr_changed_add_policy(vmhdlr_t *hdlr, policy_t *policy)
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
        fileinfo_class = hfm_read_32(context, context->regs->rsp + 5 * sizeof(addr_t));
    }
    else {
        handle = hfm_read_32(context, context->regs->rsp + 1 * sizeof(uint32_t));
        fileinfo_addr = hfm_read_32(context, context->regs->rsp + 3 * sizeof(uint32_t));
        fileinfo_class = hfm_read_32(context, context->regs->rsp + 5 * sizeof(uint32_t));
    }
    if (FILE_BASIC_INFORMATION == fileinfo_class) {
        uint32_t file_attributes = hfm_read_32(context, fileinfo_addr + handler->offsets[FILE_BASIC_INFORMATION__FILE_ATTRIBUTES]);
        char filename[STR_BUFF] = "";
        addr_t file_object = hfm_fileobj_from_handle(vmi, context, handle);
        hfm_read_filename_from_object(vmi, context, file_object, filename);
        int policy_id = filter_match(filter, filename);
        if (policy_id >= 0) {
            printf("matched\n");
        }
        if (file_attributes != 0) {
            char filename[STR_BUFF] = "";
            addr_t file_object = hfm_fileobj_from_handle(vmi, context, handle);
            hfm_read_filename_from_object(vmi, context, file_object, filename);
            int policy_id = filter_match(filter, filename);
            if (policy_id >= 0) {
                output_info_t output;
                output.pid = hfm_get_process_pid(vmi, context);
                struct timeval now;
                gettimeofday(&now, NULL);
                output.time_sec = now.tv_sec;
                output.time_usec = now.tv_usec;
                output.vmid = handler->domid;
                output.action = MON_CHANGE_ATTR;
                output.policy_id = policy_id;
                strncpy(output.filepath, filename, PATH_MAX_LEN);
                _attr_to_str(file_attributes, output.data);
                output.extpath[0] = '\0';
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

static void _attr_to_str(uint32_t attr, char *buff)
{
    int pos = 0;
    if (attr == ATTR_NORMAL) {
        buff[pos++] = 'N';
    }
    else {
        if (attr & ATTR_READ_ONLY) buff[pos++] = 'R';
        if (attr & ATTR_HIDDEN) buff[pos++] = 'H';
        if (attr & ATTR_SYSTEM) buff[pos++] = 'S';
        if (attr & ATTR_DIRECTORY) buff[pos++] = 'D';
        if (attr & ATTR_ARCHIVE) buff[pos++] = 'A';
        if (attr & ATTR_TEMPORARY) buff[pos++] = 'T';
        if (attr & ATTR_SPARSE_FILE) buff[pos++] = 'P';
        if (attr & ATTR_SYMBOLIC_LINK) buff[pos++] = 'L';
        if (attr & ATTR_ENCRYPTED) buff[pos++] = 'E';
    }
    buff[pos] = '\0';
}

void attr_changed_close(void)
{
    if (filter) filter_close(filter);
}
