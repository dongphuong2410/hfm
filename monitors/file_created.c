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

typedef struct transdata_t {
    addr_t objattr_addr;
    addr_t io_status_addr;
    char filename[STR_BUFF];
} transdata_t;

static void *syscall_cb(vmhdlr_t *handler, trap_data_t *data);
static void *sysret_cb(vmhdlr_t *handler, trap_data_t *data);
static objattr_t *_objattr_read(vmhdlr_t *handler, trap_data_t *data, addr_t addr);
static uint64_t _iostatus_read(vmhdlr_t *handler, trap_data_t *data, addr_t addr);

config_t *config;
static addr_t objattr_name;
static addr_t unicode_buff;
static addr_t iostatus_information;

int file_created_init(void)
{
    const char *rekall_profile = config_get_str(config, "rekall_profile");
    int status;
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "ObjectName", &objattr_name, NULL);
    if (status)
        goto error;
    status |= rekall_lookup(rekall_profile, "_UNICODE_STRING", "Buffer", &unicode_buff, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Information", &iostatus_information, NULL);
    //printf("iostatus %d\n", iostatus_information);
    if (status)
        goto error;
    return 0;
error:
    return -1;
}

hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy)
{
    hfm_monitor_syscall(hdlr, "NtCreateFile", syscall_cb, sysret_cb);
    return FAIL;
}

static void *syscall_cb(vmhdlr_t *handler, trap_data_t *data)
{
    addr_t attr = 0, io_status = 0;
    uint32_t create = 0;
    /* Get address of ObjectAttributes (third parameter) and IoStatusBlock (fourth parameter) */
    if (handler->pm == VMI_PM_IA32E) {
        /* For IA32E case, first 4 params will be transfered using
           register RCX, RDX, R8, R9, the remains will be transfered
           using stack */
        attr = data->regs->r8;
        io_status = data->regs->r9;
        vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
        vmi_read_32_va(vmi, data->regs->rsp + sizeof(uint32_t) * 4, 0, &create);
        hfm_release_vmi(handler);
        printf("CreateDisposition %u\n", create);
    }
    else {
        vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
        vmi_read_32_va(vmi, data->regs->rsp + sizeof(uint32_t) * 3, 0, (uint32_t *)&attr);
        vmi_read_32_va(vmi, data->regs->rsp + sizeof(uint32_t) * 4, 0, (uint32_t *)&io_status);
        vmi_read_32_va(vmi, data->regs->rsp + sizeof(uint32_t) * 8, 0, &create);
        printf("CreateDisposition %u\n", create);
        hfm_release_vmi(handler);
    }

    objattr_t *obj = _objattr_read(handler, data, attr);
    transdata_t *dt = (transdata_t *)calloc(1, sizeof(transdata_t));
    dt->objattr_addr = attr;
    dt->io_status_addr = io_status;
    if (obj) {
        strncpy(dt->filename, obj->name, STR_BUFF);
        free(obj);
    }
    return dt;
}

static void *sysret_cb(vmhdlr_t *handler, trap_data_t *data)
{
    transdata_t *dt = (transdata_t *)data->trap->extra;
    uint64_t information = _iostatus_read(handler, data, dt->io_status_addr);
    objattr_t *obj = _objattr_read(handler, data, dt->objattr_addr);
    printf("File %s with status code %lu\n", obj->name, information);
    //if (FILE_CREATED == information) {
    //    printf("File %s has just been created\n", dt->filename);
    //}
    return NULL;
}

static objattr_t *_objattr_read(vmhdlr_t *handler, trap_data_t *data, addr_t addr)
{
    objattr_t *obj = NULL;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    addr_t name = 0;

    access_context_t ctx;
    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    ctx.dtb = data->regs->cr3;

    if (!addr) {
        goto done;
    }
    ctx.addr = addr + objattr_name;
    vmi_read_addr(vmi, &ctx, &name);

    ctx.addr = name;

    uint16_t length = 0;
    status_t rc = vmi_read_16(vmi, &ctx, &length);
    if (VMI_FAILURE == rc || length > VMI_PS_4KB) {
        goto done;
    }

    unicode_string_t str, str2 = {.contents = NULL};
    str.contents = (unsigned char*)g_malloc0(length + 2);
    str.length = length;
    str.encoding = "UTF-16";

    ctx.addr = name + unicode_buff;
    rc = vmi_read_addr(vmi, &ctx, &ctx.addr);
    if (VMI_FAILURE == rc) {
        g_free(str.contents);
        goto done;
    }
    if (length != vmi_read(vmi, &ctx, str.contents, length)) {
        g_free(str.contents);
        goto done;
    }
    rc = vmi_convert_str_encoding(&str, &str2, "UTF-8");
    g_free(str.contents);

    if (VMI_SUCCESS == rc) {
        //printf("File accessed %s\n", str2.contents);
        obj = (objattr_t *)calloc(1, sizeof(objattr_t));
        strncpy(obj->name, str2.contents + 4, STR_BUFF);
        g_free(str2.contents);
        goto done;
    }
    else {
        writelog(LV_DEBUG, "Convert string encoding failed");
    }
done:
    hfm_release_vmi(handler);
    return obj;
}

static uint64_t _iostatus_read(vmhdlr_t *handler, trap_data_t *data, addr_t addr)
{
    uint64_t information = 0;
    vmi_instance_t vmi = hfm_lock_and_get_vmi(handler);
    access_context_t ctx;
    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    ctx.dtb = data->regs->cr3;

    if (!addr) {
        goto done;
    }

    ctx.addr = addr + iostatus_information;
    vmi_read_64(vmi, &ctx, &information);
done:
    hfm_release_vmi(handler);
    return information;
}
