#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "file_created.h"
#include "hfm.h"
#include "log.h"
#include "rekall.h"
#include "constants.h"
#include "context.h"
#include "file_filter.h"

#define NT_SUCCESS(status)  ((status) >= 0 && (status) <= 0x7FFFFFFF)
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
} params_t;

filter_t *filter = NULL;

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
  * Convert the _UNICODE_STRING structure into text
  * @return Len of text converted
  */
static int _read_unicode(vmi_instance_t vmi, context_t *context, addr_t unicode_str_addr, char *buffer);

/**
  * Get current directory of process
  * @return Length of path
  */
int _read_process_path(vmhdlr_t *handler, context_t *context, char *path);

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
    return FAIL;
}

void file_created_close(void)
{
    if (filter) filter_close(filter);
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

    uint64_t rootdir_addr = hfm_read_64(vmi, context, objattr_addr + OBJECT_ATTRIBUTES_ROOT_DIRECTORY);
    addr_t objectname_addr = hfm_read_addr(vmi, context, objattr_addr + OBJECT_ATTRIBUTES_OBJECT_NAME);
    char filepath[STR_BUFF] = "";
    int pathlen = 0;

    if (rootdir_addr) {
        pathlen = _read_process_path(handler, context, filepath);
    }

    int namelen = _read_unicode(vmi, context, objectname_addr, filepath + pathlen);

    //Matching file path
    if (filter_match(filter, filepath) >= 0) {
        params = (params_t *)calloc(1, sizeof(params_t));
        strncpy(params->filename, filepath, pathlen + namelen + 1);
        params->io_status_addr = io_status_addr;
        params->create_mode = create;
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
    uint64_t information = hfm_read_64(vmi, context, params->io_status_addr + IO_STATUS_BLOCK_INFORMATION);
    int status = (int)hfm_read_32(vmi, context, params->io_status_addr + IO_STATUS_BLOCK_STATUS);
    int ret_status = context->regs->rax;

    if (information == FILE_CREATED || information == FILE_SUPERSEDED && NT_SUCCESS(context->regs->rax)) {
        printf("CREATE file %s\n", params->filename);
    }
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
        fileinfo_class = hfm_read_32(vmi, context, context->regs->rsp + 5 * sizeof(addr_t));
    }
    else {
        iostatus_addr = hfm_read_32(vmi, context, context->regs->rsp + 2 * sizeof(uint32_t));
        fileinfo_addr = hfm_read_32(vmi, context, context->regs->rsp + 3 * sizeof(uint32_t));
        fileinfo_class = hfm_read_32(vmi, context, context->regs->rsp + 5 * sizeof(uint32_t));
    }
    if (FILE_RENAME_INFORMATION == fileinfo_class) {
        //Read FileName length
        addr_t filename_length_addr = fileinfo_addr + FILE_RENAME_INFORMATION_FILE_NAME_LENGTH;
        addr_t filename_addr = fileinfo_addr + FILE_RENAME_INFORMATION_FILE_NAME;
        uint32_t filename_length = hfm_read_32(vmi, context, filename_length_addr);
        if (filename_length > 0) {
            unicode_string_t str;
            str.length = filename_length;
            str.encoding = "UTF-16";
            str.contents = (unsigned char *)g_malloc0(filename_length);
            if (filename_length != hfm_read(vmi, context, filename_addr, str.contents, filename_length)) {
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

static int _read_unicode(vmi_instance_t vmi, context_t *context, addr_t unicode_str_addr, char *buffer)
{
    //TODO Rewrite the read_unicode function, not using vmi_convert_str_encoding to avoid dynamic allocation
    int ret = 0;

    //Read unicode string length
    uint16_t length = hfm_read_16(vmi, context, unicode_str_addr + UNICODE_STRING_LENGTH);
    if (0 == length || length > VMI_PS_4KB)
        goto done;

    //Read unicode string buffer address
    addr_t buffer_addr = hfm_read_addr(vmi, context, unicode_str_addr + UNICODE_STRING_BUFFER);
    if (0 == buffer_addr)
        goto done;

    unicode_string_t str, str2 = {.contents = NULL};
    str.contents = (unsigned char*)g_malloc0(length + 2);
    str.length = length;
    str.encoding = "UTF-16";

    if (length != hfm_read(vmi, context, buffer_addr, str.contents, length)) {
        g_free(str.contents);
        goto done;
    }
    status_t rc = vmi_convert_str_encoding(&str, &str2, "UTF-8");
    g_free(str.contents);

    if (VMI_SUCCESS == rc) {
        ret = str2.length;
        strncpy(buffer, str2.contents, str2.length);
        g_free(str2.contents);
        goto done;
    }
    else {
        writelog(LV_DEBUG, "Convert string encoding failed");
    }
done:
    return ret;
}

int _read_process_path(vmhdlr_t *handler, context_t *context, char *path)
{
    path[0] = '\\'; //Temporarily
    int len = 1;
    vmi_instance_t vmi = handler->vmi;
    addr_t kpcr = 0;
    if (handler->pm == VMI_PM_IA32E) {
        kpcr = context->regs->gs_base;
    }
    else {
        kpcr = context->regs->fs_base;
    }
    addr_t thread = hfm_read_addr(vmi, context, kpcr + KPCR_PRCB + KPRCB_CURRENT_THREAD);
    if (!thread) goto done;
    addr_t process = hfm_read_addr(vmi, context, thread + KTHREAD_PROCESS);
    if (!process) goto done;
    addr_t peb = hfm_read_addr(vmi, context, process + EPROCESS_PEB);
    if (!peb) goto done;
    addr_t process_parameters = hfm_read_addr(vmi, context, peb + PEB_PROCESS_PARAMETERS);
    if (!process_parameters) goto done;
    addr_t imagepath = process_parameters + RTL_USER_PROCESS_PARAMETERS_IMAGE_PATH_NAME;
    len += _read_unicode(vmi, context, imagepath, path + 1);
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

