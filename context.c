#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>

#include "context.h"
#include "constants.h"
#include "log.h"

static addr_t _get_obj_from_handle(vmi_instance_t vmi, context_t *ctx, reg_t handle);

addr_t hfm_read_addr(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    addr_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_addr(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint64_t hfm_read_64(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint64_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_64(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint32_t hfm_read_32(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint32_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_32(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint16_t hfm_read_16(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint16_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_16(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint8_t hfm_read_8(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint8_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_8(vmi, &ctx->access_ctx, &ret);
    return ret;
}

size_t hfm_read(vmi_instance_t vmi, context_t *ctx, addr_t addr, void *buf, size_t count)
{
    ctx->access_ctx.addr = addr;
    return vmi_read(vmi, &ctx->access_ctx, buf, count);
}

addr_t hfm_get_current_process(vmi_instance_t vmi, context_t *ctx)
{
    addr_t process = 0;
    addr_t kpcr = 0, prcb = 0;
    if (ctx->pm == VMI_PM_IA32E) {
        kpcr = ctx->regs->gs_base;
        prcb = KPCR_PRCB;
    }
    else {
        kpcr = ctx->regs->fs_base;
        prcb = KPCR_PRCB_DATA;
    }
    addr_t thread = hfm_read_addr(vmi, ctx, kpcr + prcb + KPRCB_CURRENT_THREAD);
    if (!thread) goto done;
    process = hfm_read_addr(vmi, ctx, thread + KTHREAD_PROCESS);
done:
    return process;
}

int hfm_read_filename_from_handler(vmi_instance_t vmi, context_t *ctx, reg_t handle, char *filename)
{
    int filepath_len = 0;
    int drivename_len = 0;

    /* Find the file object from the handle number*/
    addr_t handle_obj = _get_obj_from_handle(vmi, ctx, handle);
    uint8_t type_index = hfm_read_8(vmi, ctx, handle_obj + OBJECT_HEADER_TYPE_INDEX);
    if (type_index >= WIN7_TYPEINDEX_LAST || type_index != 28) goto done;
    addr_t file_object = handle_obj + OBJECT_HEADER_BODY;

    /* Find the drive label (device name) from file object */
    char drivename[STR_BUFF] = "";
    addr_t device_object = hfm_read_addr(vmi, ctx, file_object + FILE_OBJECT_DEVICE_OBJECT);
    addr_t device_header = device_object - OBJECT_HEADER_BODY;
    uint8_t infomask = hfm_read_8(vmi, ctx, device_header + OBJECT_HEADER_INFO_MASK);
    addr_t device_name_info_offset = 0;
    if (infomask & OB_INFOMASK_CREATOR_INFO) {
        device_name_info_offset += 0x20;        //TODO 0x20 = sizeof struct OBJECT_HEADER_CREATOR_INFO, should remove hardcode
    }
    if (infomask & OB_INFOMASK_NAME) {
        device_name_info_offset += 0x20;        //TODO Here, 0x20 = sizeof struct OBJECT_HEADER_NAME_INFO, should remove hardcode
        addr_t device_name_info_addr = device_header - device_name_info_offset;
        hfm_read_unicode(vmi, ctx, device_name_info_addr + OBJECT_HEADER_NAME_INFO_NAME, drivename);
        //TODO: hardcode mapping Windows Device Name to Drive Label
        if (!strncmp(drivename, "HarddiskVolume2", STR_BUFF)) {
            sprintf(drivename, "%s", "C:");
            drivename_len = 2;
        }
    }

    /* Find filepath from file object */
    char filepath[STR_BUFF] = "";
    filepath_len = hfm_read_unicode(vmi, ctx, file_object + FILE_OBJECT_FILE_NAME, filepath);

    /* Return the full path read : path = drivename + filepath */
    snprintf(filename, STR_BUFF, "%s%s", drivename, filepath);
done:
    return drivename_len + filepath_len;
}

int hfm_read_unicode(vmi_instance_t vmi, context_t *ctx, addr_t addr, char *buffer)
{
    //TODO Rewrite the read_unicode function, not using vmi_convert_str_encoding to avoid dynamic allocation
    int ret = 0;

    //Read unicode string length
    uint16_t length = hfm_read_16(vmi, ctx, addr + UNICODE_STRING_LENGTH);
    if (0 == length || length > VMI_PS_4KB)
        goto done;

    //Read unicode string buffer address
    addr_t buffer_addr = hfm_read_addr(vmi, ctx, addr + UNICODE_STRING_BUFFER);
    if (0 == buffer_addr)
        goto done;

    unicode_string_t str, str2 = {.contents = NULL};
    str.contents = (unsigned char*)g_malloc0(length + 2);
    str.length = length;
    str.encoding = "UTF-16";

    if (length != hfm_read(vmi, ctx, buffer_addr, str.contents, length)) {
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

static addr_t _get_obj_from_handle(vmi_instance_t vmi, context_t *ctx, reg_t handle)
{
    addr_t handle_obj = 0;
    addr_t process = hfm_get_current_process(vmi, ctx);
    if (!process) goto done;
    addr_t handletable = hfm_read_addr(vmi, ctx, process + EPROCESS_OBJECT_TABLE);
    if (!handletable) goto done;
    addr_t tablecode = hfm_read_addr(vmi, ctx, handletable + HANDLE_TABLE_TABLE_CODE);
    if (!tablecode) goto done;
    uint32_t handlecount = hfm_read_32(vmi, ctx,handletable + HANDLE_TABLE_HANDLE_COUNT);
    if (!handlecount) goto done;
    addr_t table_base = tablecode & ~EX_FAST_REF_MASK;
    uint32_t table_levels = tablecode & EX_FAST_REF_MASK;

    reg_t handle_idx = handle / HANDLE_MULTIPLIER;
    switch (table_levels) {
        case 0:
            handle_obj = hfm_read_addr(vmi, ctx, table_base + handle_idx * HANDLE_TABLE_ENTRY_SIZE);
            break;
        case 1:
        {
            addr_t table = 0;
            size_t psize = (ctx->pm == VMI_PM_IA32E ? 8 : 4);
            uint32_t lowest_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;   //Number of handle entry in the lowest level table
            uint32_t i = handle_idx % lowest_count;
            handle_idx -= i;
            uint32_t j = handle_idx / lowest_count;
            table = hfm_read_addr(vmi, ctx, table_base + j * psize);
            if (table) {
                handle_obj = hfm_read_addr(vmi, ctx, table + i * HANDLE_TABLE_ENTRY_SIZE);
            }
            break;
        }
        case 2:
        {
            addr_t table = 0, table2 = 0;
            size_t psize = (ctx->pm == VMI_PM_IA32E ? 8 : 4);
            uint32_t lowest_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;
            uint32_t mid_count = VMI_PS_4KB / psize;
            uint32_t i = handle_idx % lowest_count;
            handle_idx -= i;
            uint32_t j = handle_idx / lowest_count;
            uint32_t k = j % mid_count;
            j = (j - k)/mid_count;
            table = hfm_read_addr(vmi, ctx, table_base + j * psize);
            if (table)
                table2 = hfm_read_addr(vmi, ctx, table + k * psize);
            if (table2)
                handle_obj = hfm_read_addr(vmi, ctx, table2 + i * HANDLE_TABLE_ENTRY_SIZE);
            break;
        }
    }
    switch (ctx->winver) {
        case VMI_OS_WINDOWS_7:
            handle_obj &= ~EX_FAST_REF_MASK;
            break;
        case VMI_OS_WINDOWS_8:
            if (ctx->pm == VMI_PM_IA32E)
                handle_obj = ((handle_obj & VMI_BIT_MASK(19,63)) >> 16) | 0xFFFFE00000000000;
            else
                handle_obj &= VMI_BIT_MASK(2,31);
            break;
        default:
        case VMI_OS_WINDOWS_10:
            if (ctx->pm == VMI_PM_IA32E)
                handle_obj = ((handle_obj & VMI_BIT_MASK(19,63)) >> 16) | 0xFFFF000000000000;
            else
                handle_obj &= VMI_BIT_MASK(2,31);
            break;
    }
done:
    return handle_obj;
}

vmi_pid_t hfm_get_process_pid(vmi_instance_t vmi, context_t *ctx)
{
    vmi_pid_t pid;
    if (!ctx->process_base) {
        ctx->process_base = hfm_get_current_process(vmi, ctx);
    }
    ctx->access_ctx.addr = ctx->process_base + EPROCESS_UNIQUE_PROCESS_ID;
    if (VMI_SUCCESS != vmi_read_32(vmi, &ctx->access_ctx, &pid)) {
        writelog(LV_ERROR, "Failed to get the pid of process %p", ctx->process_base);
        pid = -1;
    }
    return pid;
}
