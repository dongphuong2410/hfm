#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "file_created.h"
#include "hfm.h"
#include "log.h"
#include "config.h"
#include "rekall.h"
#include "constants.h"
#include "context.h"

typedef struct objattr_t {
    char name[STR_BUFF];
} objattr_t;

typedef struct params_t {
    addr_t io_status_addr;
    char filename[STR_BUFF];
} params_t;

static void *syscall_cb(vmhdlr_t *handler, context_t *context);
static void *sysret_cb(vmhdlr_t *handler, context_t *context);
static char *_read_unicode(vmi_instance_t vmi, context_t *context, addr_t addr);

config_t *config;
static addr_t OBJEC_ATTRIBUTES_OBJECT_NAME;
static addr_t UNICODE_STRING_LENGTH;
static addr_t UNICODE_STRING_BUFFER;
static addr_t IO_STATUS_BLOCK_INFORMATION;
static addr_t IO_STATUS_BLOCK_STATUS;

int file_created_init(void)
{
    const char *rekall_profile = config_get_str(config, "rekall_profile");
    int status = 0;
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "ObjectName", &OBJEC_ATTRIBUTES_OBJECT_NAME, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "Length", &UNICODE_STRING_LENGTH, NULL);
    status |= rekall_lookup(rekall_profile, "_UNICODE_STRING", "Buffer", &UNICODE_STRING_BUFFER, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Information", &IO_STATUS_BLOCK_INFORMATION, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Status", &IO_STATUS_BLOCK_STATUS, NULL);
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

static void *syscall_cb(vmhdlr_t *handler, context_t *context)
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

static void *sysret_cb(vmhdlr_t *handler, context_t *context)
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

/**
  * Read Filename from OBJECT_ATTRIBUTES struct
  */
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
