#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>

#include "context.h"
#include "constants.h"
#include "log.h"

static addr_t handle_table_get_entry(uint32_t bit, vmi_instance_t vmi,
        addr_t table_base, uint32_t level, uint32_t depth,
        uint32_t *handle_count, uint64_t handle);

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
    addr_t kpcr = 0;
    if (ctx->pm == VMI_PM_IA32E) {
        kpcr = ctx->regs->gs_base;
    }
    else {
        kpcr = ctx->regs->fs_base;
    }
    addr_t thread = hfm_read_addr(vmi, ctx, kpcr + KPCR_PRCB + KPRCB_CURRENT_THREAD);
    if (!thread) goto done;
    process = hfm_read_addr(vmi, ctx, thread + KTHREAD_PROCESS);
done:
    return process;
}

int hfm_read_filename_from_handler(vmi_instance_t vmi, context_t *ctx, reg_t handle, char *filename)
{
    int filename_len = 0;
    int drive_len = 0;
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
    uint32_t table_depth = 0;
    addr_t obj = handle_table_get_entry(PM2BIT(ctx->pm), vmi, table_base, table_levels, table_depth, &handlecount, handle);
    if (!obj) goto done;

    uint8_t type_index = hfm_read_8(vmi, ctx, obj + OBJECT_HEADER_TYPE_INDEX);
    if (type_index >= WIN7_TYPEINDEX_LAST || type_index != 28) goto done;

    addr_t file_object = obj + OBJECT_HEADER_BODY;

    //Read device name
    addr_t device_object = hfm_read_addr(vmi, ctx, file_object + FILE_OBJECT_DEVICE_OBJECT);
    addr_t device_name_info_offset = OBJECT_HEADER_BODY;
    addr_t device_object_header = device_object - OBJECT_HEADER_BODY;
    char device_name[STR_BUFF] = "";
    uint8_t infomask = hfm_read_8(vmi, ctx, device_object_header + OBJECT_HEADER_INFO_MASK);
    if (infomask & OB_INFOMASK_CREATOR_INFO) {
        device_name_info_offset += 0x20;        //TODO 0x20 = sizeof struct OBJECT_HEADER_CREATOR_INFO, should remove hardcode
    }
    if (infomask & OB_INFOMASK_NAME) {
        device_name_info_offset += 0x20;        //TODO Here, 0x20 = sizeof struct OBJECT_HEADER_NAME_INFO, should remove hardcode
        addr_t device_name_info_addr = device_object - device_name_info_offset;
        hfm_read_unicode(vmi, ctx, device_name_info_addr + OBJECT_HEADER_NAME_INFO_NAME, device_name);
        //TODO: hardcode mapping Windows Device Name to Drive Label
        if (!strncmp(device_name, "HarddiskVolume2", STR_BUFF)) {
            sprintf(filename, "%s", "C:");
            drive_len = 2;
        }
    }

    addr_t filename_addr = file_object + FILE_OBJECT_FILE_NAME;
    filename_len = hfm_read_unicode(vmi, ctx, filename_addr, filename + drive_len);
done:
    return drive_len + filename_len;
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

static addr_t handle_table_get_entry(uint32_t bit, vmi_instance_t vmi,
        addr_t table_base, uint32_t level, uint32_t depth,
        uint32_t *handle_count, uint64_t handle) {

    uint32_t count;
    uint32_t table_entry_size = 0;

    if (level > 0) {
        if (bit == BIT32)
            table_entry_size = 0x4;
        else
            table_entry_size = 0x8;
    } else if (level == 0) {
        if (bit == BIT32)
            table_entry_size = 0x8;
        else
            table_entry_size = 0x10;
    };
    count = VMI_PS_4KB / table_entry_size;
    uint32_t i;
    for (i = 0; i < count; i++) {

        // Only read the already known number of entries
        if (*handle_count == 0)
            break;

        addr_t table_entry_addr;
        if ( VMI_FAILURE == vmi_read_addr_va(vmi, table_base + i * table_entry_size, 0, &table_entry_addr) )
            continue;

        // skip entries that point nowhere
        if (!table_entry_addr)
            continue;

        // real entries are further down the chain
        if (level > 0) {
            addr_t next_level;
            if ( VMI_FAILURE == vmi_read_addr_va(vmi, table_base + i * table_entry_size, 0, &next_level) )
                continue;

            addr_t test = handle_table_get_entry(bit, vmi, next_level, level - 1,
                                                 depth, handle_count, handle);
            if (test)
                return test;

            depth++;
            continue;
        }

        // At this point each (table_base + i*entry) is a _HANDLE_TABLE_ENTRY

        uint32_t level_base = depth * count * HANDLE_MULTIPLIER;
        uint32_t handle_value = (i * table_entry_size * HANDLE_MULTIPLIER)
                / table_entry_size + level_base;

        if (handle_value == handle)
            return table_entry_addr & ~EX_FAST_REF_MASK;

        // decrement the handle counter because we found one here
        --(*handle_count);
    }
    return 0;
}

