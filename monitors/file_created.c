#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include "monitor.h"
#include "hfm.h"
#include "log.h"
#include "constants.h"
#include "context.h"
#include "file_filter.h"
#include "output_format.h"

/**
  * typedef struct _UNICODE_STRING {
  *     USHORT Length;
  *     USHORT MaximumLength;
  *     PWSTR  Buffer;
  * } UNICODE_STRING, *PUNICODE_STRING
  */

typedef struct params_t {
    addr_t io_status_addr;
    addr_t handler_addr;
    char filename[STR_BUFF];
    uint32_t create_mode;
    int policy_id;
} params_t;

static filter_t *filter = NULL;

/**
  * Callback when the functions NtOpenFile, NtCreateFile, ZwOpenFile, ZwCreateFile is called
  */
static void *createfile_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtOpenFile, NtCreateFile, ZwOpenFile, ZwCreateFile is return
  */
static void *createfile_ret_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is called
  */
static void *setinformation_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtSetInformatonFile, ZwSetInformationFile is returned
  */
static void *setinformation_ret_cb(vmhdlr_t *handler, context_t *context);

/**
  * Get current directory of process
  * @return Length of path
  */
static int _read_process_path(vmi_instance_t vmi, context_t *context, char *path);

hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    if (!filter) {
        //Init the plugin
        filter = filter_init();
        hfm_monitor_syscall(hdlr, "NtOpenFile", createfile_cb, createfile_ret_cb);
        hfm_monitor_syscall(hdlr, "NtCreateFile", createfile_cb, createfile_ret_cb);
        hfm_monitor_syscall(hdlr, "NtSetInformationFile", setinformation_cb, setinformation_ret_cb);
        hfm_monitor_syscall(hdlr, "ZwSetInformationFile", setinformation_cb, setinformation_ret_cb);
    }
    filter_add(filter, policy->path, policy->id);
    return SUCCESS;
}

void file_created_close(void)
{
    if (filter) {
        filter_close(filter);
        filter = NULL;
    }
}
/**
  * Callback when NtCreateFile or NtOpenFile,.. is called
  * We will read the ObjectAttributes for ObjectName (filename)
  * Read the address of IoStatusBlock and transfered to createfile_ret_cb
  *
  * NTSTATUS NtCreateFile(
  *     _Out_   PHANDLE             FileHandle,
  *     _In_    ACCESS_MASK         DesiredAccess,
  *     _In_    POBJECT_ATTRIBUTES  ObjectAttributes,
  *     _Out_   PIO_STATUS_BLOCK    IoStatusBlock,
  *     ....
  * );
  *
  * typedef _OBJECT_ATTRIBUTES {
  *     ULONG               Length;
  *     HANDLE              RootDirectory;
  *     PUNICODE_STRING     ObjectName;
  *     ULONG               Attributes;
  *     PVOID               SecurityDescriptor;
  *     PVOID               SecurityQualityOfService
  * } OBJECT_ATTRIBUTES;
  */
static void *createfile_cb(vmhdlr_t *handler, context_t *context)
{
    addr_t objattr_addr = 0, io_status_addr = 0;
    uint32_t create = 0;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    params_t *params = NULL;

    /* Get address of ObjectAttributes (third parameter) and IoStatusBlock (fourth parameter) */
    if (handler->pm == VMI_PM_IA32E) {
        /* For IA32E case, first 4 params will be transfered using
           register RCX, RDX, R8, R9, the remains will be transfered
           using stack */
        objattr_addr = context->regs->r8;
        io_status_addr = context->regs->r9;
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 4, 0, &create);
    }
    else {
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 3, 0, (uint32_t *)&objattr_addr);
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 4, 0, (uint32_t *)&io_status_addr);
        vmi_read_32_va(vmi, context->regs->rsp + sizeof(uint32_t) * 8, 0, &create);
    }

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
        params = (params_t *)calloc(1, sizeof(params_t));
        strncpy(params->filename, start_filepath, pathlen + namelen + 1);
        params->io_status_addr = io_status_addr;
        params->create_mode = create;
        params->policy_id = policy_match;
    }
    hfm_release_vmi(handler);
    return params;
}

/**
  * Callback when NtCreateFile, NtOpenFile, ZwCreateFile, ZwOpenFile ..
  * is returned.
  * We received the io_status_block address from createfile_cb through the passing params
  * Read the IO_STATUS_BLOCK for Information field and Status field
  * A file is newly created when the Information has value FILE_CREATED and status STATUS_SUCCESS
  *
  * typedef struct _IO_STATUS_BLOCK {
  *     union {
  *         NTSTATUS Status;
  *         PVOID    Pointer;
  *     };
  *     ULONG_PTR Information;
  * } IO_STATUS_BLOCK;
  */
static void *createfile_ret_cb(vmhdlr_t *handler, context_t *context)
{
    params_t *params = (params_t *)context->trap->extra;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    uint64_t information = hfm_read_64(context, params->io_status_addr + context->hdlr->offsets[IO_STATUS_BLOCK__INFORMATION]);
    int status = (int)hfm_read_32(context, params->io_status_addr + context->hdlr->offsets[IO_STATUS_BLOCK__STATUS]);
    int ret_status = context->regs->rax;

    if (information == FILE_CREATED || information == FILE_SUPERSEDED && NT_SUCCESS(context->regs->rax)) {
        output_info_t output;
        output.pid = hfm_get_process_pid(vmi, context);
        struct timeval now;
        gettimeofday(&now, NULL);
        output.time_sec = now.tv_sec;
        output.time_usec = now.tv_usec;
        output.vmid = handler->domid;
        output.action = MON_CREATE;
        output.policy_id = params->policy_id;
        strncpy(output.filepath, params->filename, PATH_MAX_LEN);
        output.extpath[0] = '\0';
        output.data[0] = '\0';
        out_write(handler->out, &output);
    }
    free(params);
done:
    hfm_release_vmi(handler);
    return NULL;
}

/**
  * NTSTATUS ZwSetInformationFile(
  *     _In_  HANDLE                    FileHandle,
  *     _Out_ PIO_STATUS_BLOCK          IoStatusBlock,
  *     _In_  PVOID                     FileInformation,
  *     _In_  ULONG                     Length,
  *     _In_  FILE_INFORMATION_CLASS    FileInformationClass
  * );
  *
  * typedef struct _FILE_RENAME_INFORMATION {
  *     BOOLEAN     ReplaceIfExists;
  *     HANDLE      RootDirectory;
  *     ULONG       FileNameLength;
  *     WCHAR       FileName[1];
  * } FILE_RENAME_INFORMATION;
  *
  * - Read FileInformationClass, check if class is FILE_RENAME_INFORMATION
  * - Read FILE_RENAME_INFORMATION struct for new filename, transfered to setinformation_ret_cb
  * - Read the IoStatusBlock, transfered to setinformation_ret_cb
  */
static void *setinformation_cb(vmhdlr_t *handler, context_t *context)
{
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    addr_t fileinfo_addr = 0;
    uint32_t fileinfo_class = 0;
    addr_t iostatus_addr = 0;

    //Read FileInformationClass and address of FileInformation
    if (handler->pm == VMI_PM_IA32E) {
        iostatus_addr = context->regs->rdx;
        fileinfo_addr = context->regs->r8;
        fileinfo_class = hfm_read_32(context, context->regs->rsp + 5 * sizeof(addr_t));
    }
    else {
        iostatus_addr = hfm_read_32(context, context->regs->rsp + 2 * sizeof(uint32_t));
        fileinfo_addr = hfm_read_32(context, context->regs->rsp + 3 * sizeof(uint32_t));
        fileinfo_class = hfm_read_32(context, context->regs->rsp + 5 * sizeof(uint32_t));
    }
    if (FILE_RENAME_INFORMATION == fileinfo_class) {
        //Read FileName length
        addr_t filename_length_addr = fileinfo_addr + context->hdlr->offsets[FILE_RENAME_INFORMATION__FILE_NAME_LENGTH];
        addr_t filename_addr = fileinfo_addr + context->hdlr->offsets[FILE_RENAME_INFORMATION__FILE_NAME];
        uint32_t filename_length = hfm_read_32(context, filename_length_addr);
        if (filename_length > 0) {
            unicode_string_t str;
            str.length = filename_length;
            str.encoding = "UTF-16";
            str.contents = (unsigned char *)g_malloc0(filename_length);
            if (filename_length != hfm_read(context, filename_addr, str.contents, filename_length)) {
                free(str.contents);
                goto done;
            }
            unicode_string_t str2 = { .contents = NULL };
            if (VMI_SUCCESS == vmi_convert_str_encoding(&str, &str2, "UTF-8")) {
                printf("RENAME %s\n", str2.contents);
                free(str2.contents);
            }
            else {
                printf("RENAME , filename converted failed\n");
            }
            free(str.contents);
        }
    }
done:
    hfm_release_vmi(handler);
}

static void *setinformation_ret_cb(vmhdlr_t *handler, context_t *context)
{
    return NULL;
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

