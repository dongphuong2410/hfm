#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "file_created.h"
#include "hfm.h"
#include "log.h"
#include "config.h"
#include "rekall.h"
#include "constants.h"

typedef struct objattr_t {
    char name[STR_BUFF];
} objattr_t;

typedef struct params_t {
    addr_t io_status_addr;
    char filename[STR_BUFF];
} params_t;

static void *syscall_cb(vmhdlr_t *handler, const trap_context_t *context);
static void *sysret_cb(vmhdlr_t *handler, const trap_context_t *context);
static char *_read_unicode(vmi_instance_t vmi, const trap_context_t *context, addr_t addr);

config_t *config;
static addr_t OFFSET_OBJECT_ATTRIBUTES_ObjectName;
static addr_t OFFSET_UNICODE_STRING_Length;
static addr_t OFFSET_UNICODE_STRING_Buffer;
static addr_t OFFSET_IO_STATUS_BLOCK_Information;
static addr_t OFFSET_IO_STATUS_BLOCK_Status;

int file_created_init(void)
{
    const char *rekall_profile = config_get_str(config, "rekall_profile");
    int status = 0;
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "ObjectName", &OFFSET_OBJECT_ATTRIBUTES_ObjectName, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "Length", &OFFSET_UNICODE_STRING_Length, NULL);
    status |= rekall_lookup(rekall_profile, "_UNICODE_STRING", "Buffer", &OFFSET_UNICODE_STRING_Buffer, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Information", &OFFSET_IO_STATUS_BLOCK_Information, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Status", &OFFSET_IO_STATUS_BLOCK_Status, NULL);
    if (status)
        goto error;
    return 0;
error:
    return -1;
}

hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtOpenFile", syscall_cb, sysret_cb);
    hfm_monitor_syscall(hdlr, "NtCreateFile", syscall_cb, sysret_cb);
    return FAIL;
}

static void *syscall_cb(vmhdlr_t *handler, const trap_context_t *context)
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
    addr_t objectname_addr;
    access_context_t ctx;
    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    ctx.dtb = context->regs->cr3;
    ctx.addr = objattr_addr + OFFSET_OBJECT_ATTRIBUTES_ObjectName;
    vmi_read_addr(vmi, &ctx, &objectname_addr);
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

static void *sysret_cb(vmhdlr_t *handler, const trap_context_t *context)
{
    params_t *params = (params_t *)context->trap->extra;
    uint64_t information = 0;
    uint32_t status;

    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    access_context_t ctx;
    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    ctx.dtb = context->regs->cr3;

    ctx.addr = params->io_status_addr + OFFSET_IO_STATUS_BLOCK_Information;
    vmi_read_64(vmi, &ctx, &information);

    ctx.addr = params->io_status_addr + OFFSET_IO_STATUS_BLOCK_Status;
    vmi_read_32(vmi, &ctx, &status);

    if (information == FILE_CREATED && status == STATUS_SUCCESS) {
        printf("File %s\n", params->filename);
    }
done:
    hfm_release_vmi(handler);
    return NULL;
}

/**
  * Read Filename from OBJECT_ATTRIBUTES struct
  */
static char *_read_unicode(vmi_instance_t vmi, const trap_context_t *context, addr_t unicode_str_addr)
{
    char *ret = NULL;

    access_context_t ctx;
    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    ctx.dtb = context->regs->cr3;

    //Read unicode string length
    uint16_t length = 0;
    ctx.addr = unicode_str_addr + OFFSET_UNICODE_STRING_Length;
    status_t rc = vmi_read_16(vmi, &ctx, &length);
    if (VMI_FAILURE == rc || length > VMI_PS_4KB) {
        goto done;
    }

    //Read unicode string buffer address
    addr_t buffer_addr;
    ctx.addr = unicode_str_addr + OFFSET_UNICODE_STRING_Buffer;
    rc = vmi_read_addr(vmi, &ctx, &buffer_addr);
    if (VMI_FAILURE == rc) {
        goto done;
    }

    unicode_string_t str, str2 = {.contents = NULL};
    str.contents = (unsigned char*)g_malloc0(length + 2);
    str.length = length;
    str.encoding = "UTF-16";

    ctx.addr = buffer_addr;
    if (length != vmi_read(vmi, &ctx, str.contents, length)) {
        g_free(str.contents);
        goto done;
    }
    rc = vmi_convert_str_encoding(&str, &str2, "UTF-8");
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
