#include "monitor.h"
#include "file_filter.h"
#include "context.h"
#include "private.h"
#include "hfm.h"
#include "constants.h"
#include "config.h"

#define DESIRED_ACCESS_DELETE       0x00010000
#define FILE_DELETE_ON_CLOSE        0x00004000

extern config_t *config;
static filter_t *filter = NULL;

static int _read_process_path(vmi_instance_t vmi, context_t *context, char *path);
/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is called
  */
static void *setinformation_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtCreateFile, ZwCreateFile is called
  */
static void *createfile_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is returned
  */
static void *setinformation_ret_cb(vmhdlr_t *handler, context_t *context);

hfm_status_t file_deleted_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    if (!filter) {
        //Init plugin
        filter = filter_init();
        if (hdlr->winver == VMI_OS_WINDOWS_8) {
            //TODO: Monitor NtCreateFile will trigger as soon as the user create "DELETE" command, but it does not mean file will be deleted right at that time
            hfm_monitor_syscall(hdlr, "NtCreateFile", createfile_cb, NULL);
            hfm_monitor_syscall(hdlr, "ZwCreateFile", createfile_cb, NULL);
        }
        else {
            hfm_monitor_syscall(hdlr, "NtSetInformationFile", setinformation_cb, setinformation_ret_cb);
            hfm_monitor_syscall(hdlr, "ZwSetInformationFile", setinformation_cb, setinformation_ret_cb);
        }
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
        fileinfo_class = hfm_read_32(context, context->regs->rsp + 5 * sizeof(addr_t));
    }
    else {
        handle = hfm_read_32(context, context->regs->rsp + 1 * sizeof(uint32_t));
        fileinfo_addr = hfm_read_32(context, context->regs->rsp + 3 * sizeof(uint32_t));
        fileinfo_class = hfm_read_32(context, context->regs->rsp + 5 * sizeof(uint32_t));
    }
    if (FILE_DISPOSITION_INFORMATION == fileinfo_class) {
        char filename[STR_BUFF] = "";
        addr_t file_object = hfm_fileobj_from_handle(vmi, context, handle);
        hfm_read_filename_from_object(vmi, context, file_object, filename);
        int policy_id = filter_match(filter, filename);
        if (policy_id >= 0) {
            uint8_t delete = hfm_read_8(context, fileinfo_addr + context->hdlr->offsets[FILE_DISPOSITION_INFORMATION__DELETE_FILE]);
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
                output.data[0] = '\0';
                out_write(handler->out, &output);
            }
        }
    }
done:
    hfm_release_vmi(handler);
    return NULL;
}

//TODO : too slow
static void *createfile_cb(vmhdlr_t *handler, context_t *context)
{
    addr_t objattr_addr = 0, io_status_addr = 0;
    uint32_t create = 0;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    uint32_t desired_access = 0;
    uint32_t create_options = 0;

    /* Get address of ObjectAttributes (third parameter) and IoStatusBlock (fourth parameter) */
    if (handler->pm == VMI_PM_IA32E) {
        /* For IA32E case, first 4 params will be transfered using
           register RCX, RDX, R8, R9, the remains will be transfered
           using stack */
        objattr_addr = context->regs->r8;
        io_status_addr = context->regs->r9;
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 4, 0, &create);
        desired_access = context->regs->rdx;
        create_options = hfm_read_32(context, context->regs->rsp + 9 * sizeof(addr_t));
    }
    else {
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 3, 0, (uint32_t *)&objattr_addr);
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 4, 0, (uint32_t *)&io_status_addr);
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 8, 0, &create);
        desired_access = hfm_read_32(context, context->regs->rsp + 2 * sizeof(int32_t));
        create_options = hfm_read_32(context, context->regs->rsp + 9 * sizeof(int32_t));
    }

    printf("desired_access %x create_options %x\n", desired_access, create_options);
    if (desired_access & DESIRED_ACCESS_DELETE
            && create_options & FILE_DELETE_ON_CLOSE) {
        printf("DELETE\n");
        uint64_t rootdir_addr = hfm_read_64(context, objattr_addr + context->hdlr->offsets[OBJECT_ATTRIBUTES__ROOT_DIRECTORY]);
        addr_t objectname_addr = hfm_read_addr(context, objattr_addr + context->hdlr->offsets[OBJECT_ATTRIBUTES__OBJECT_NAME]);
        char filepath[STR_BUFF] = "";
        int pathlen = 0;

        if (rootdir_addr) {
            pathlen = _read_process_path(vmi, context, filepath);
        }

        int namelen = hfm_read_unicode(vmi, context, objectname_addr, filepath + pathlen);

        char *start_filepath = filepath;
        if (strstr(start_filepath, "\\??\\")) {
            start_filepath += 4;
        }
        //Matching file path
        int policy_match = filter_match(filter, start_filepath);
        if (policy_match >= 0) {
            printf("DELETE\n");
            output_info_t output;
            output.pid = hfm_get_process_pid(vmi, context);
            struct timeval now;
            gettimeofday(&now, NULL);
            output.time_sec = now.tv_sec;
            output.time_usec = now.tv_usec;
            output.vmid = handler->domid;
            output.action = MON_CREATE;
            output.policy_id = policy_match;
            strncpy(output.filepath, start_filepath, PATH_MAX_LEN);
            output.extpath[0] = '\0';
            output.data[0] = '\0';
            out_write(handler->out, &output);
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

void file_deleted_close(void)
{
    if (filter) filter_close(filter);
}

static int _read_process_path(vmi_instance_t vmi, context_t *context, char *path)
{
    int len = 0;
    addr_t process = hfm_get_current_process(vmi, context);
    if (!process) goto done;
    addr_t peb = hfm_read_addr(context, process + context->hdlr->offsets[EPROCESS__PEB]);
    if (!peb) goto done;
    addr_t process_parameters = hfm_read_addr(context, peb + context->hdlr->offsets[PEB__PROCESS_PARAMETERS]);
    if (!process_parameters) goto done;
    len = 1;
    addr_t imagepath = process_parameters + context->hdlr->offsets[RTL_USER_PROCESS_PARAMETERS__IMAGE_PATH_NAME];
    len += hfm_read_unicode(vmi, context, imagepath, path);
    if (len) {
        char *pos = strrchr(path, '\\');
        if (pos && pos != path) {
            *(pos+1) = '\0';
            len = pos + 1 - path;
        }
    }
done:
    return len;
}

