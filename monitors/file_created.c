#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "file_created.h"
#include "hfm.h"
#include "log.h"
#include "rekall.h"
#include "constants.h"
#include "context.h"

/**
  * typedef struct _UNICODE_STRING {
  *     USHORT Length;
  *     USHORT MaximumLength;
  *     PWSTR  Buffer;
  * } UNICODE_STRING, *PUNICODE_STRING
  */

typedef struct params_t {
    addr_t io_status_addr;
    char filename[STR_BUFF];
} params_t;

/**
  * Callback when the functions NtOpenFile, NtCreateFile, ZwOpenFile, ZwCreateFile is called
  */
static void *createfile_cb(vmhdlr_t *handler, context_t *context);

/**
  * Callback when the functions NtOpenFile, NtCreateFile, ZwOpenFile, ZwCreateFile is return
  */
static void *createfile_ret_cb(vmhdlr_t *handler, context_t *context);

/**
  * Convert the _UNICODE_STRING structure into text
  */
hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtOpenFile", createfile_cb, createfile_ret_cb);
    hfm_monitor_syscall(hdlr, "NtCreateFile", createfile_cb, createfile_ret_cb);
    return FAIL;
}

static void *createfile_cb(vmhdlr_t *handler, context_t *context)
{
    addr_t objattr_addr = 0, io_status_addr = 0;
    uint32_t create = 0;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);

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

    addr_t objectname_addr = hfm_read_addr(vmi, context, objattr_addr + OBJEC_ATTRIBUTES_OBJECT_NAME);
    char *filename = _read_unicode(vmi, context, objectname_addr);

    params_t *params = (params_t *)calloc(1, sizeof(params_t));
    params->io_status_addr = io_status_addr;
    if (filename) {
        strncpy(params->filename, filename, STR_BUFF);
        free(filename);
    }

    hfm_release_vmi(handler);
    return params;
}

static void *createfile_ret_cb(vmhdlr_t *handler, context_t *context)
{
    params_t *params = (params_t *)context->trap->extra;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);

    uint64_t information = hfm_read_64(vmi, context, params->io_status_addr + IO_STATUS_BLOCK_INFORMATION);

    uint32_t status = hfm_read_32(vmi, context, params->io_status_addr + IO_STATUS_BLOCK_STATUS);

    if (information == FILE_CREATED && status == STATUS_SUCCESS) {
        printf("File %s\n", params->filename);
    }
done:
    hfm_release_vmi(handler);
    return NULL;
}

static char *_read_unicode(vmi_instance_t vmi, context_t *context, addr_t unicode_str_addr)
{
    char *ret = NULL;

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
        ret = strdup(str2.contents + 4);
        g_free(str2.contents);
        goto done;
    }
    else {
        writelog(LV_DEBUG, "Convert string encoding failed");
    }
done:
    return ret;
}
