#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>
#include <libvmi/x86.h>

#include "context.h"
#include "constants.h"
#include "log.h"
#include "win.h"

/**
  * extract file
  * return 0 if success, 1 if fail
  */
static int _extract_ca_file(vmi_instance_t vmi, context_t *ctx, addr_t control_area, char *path);

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
    if (ctx->hdlr->pm == VMI_PM_IA32E) {
        kpcr = ctx->regs->gs_base;
        prcb = ctx->hdlr->offsets[KPCR__PRCB];
    }
    else {
        kpcr = ctx->regs->fs_base;
        prcb = ctx->hdlr->offsets[KPCR__PRCB_DATA];
    }
    addr_t thread = hfm_read_addr(vmi, ctx, kpcr + prcb + ctx->hdlr->offsets[KPRCB__CURRENT_THREAD]);
    if (!thread) goto done;
    if (ctx->hdlr->winver == VMI_OS_WINDOWS_XP) {
        process = hfm_read_addr(vmi, ctx, thread + ctx->hdlr->offsets[KTHREAD__APC_STATE] + ctx->hdlr->offsets[KAPC_STATE__PROCESS]);
    }
    else {
        process = hfm_read_addr(vmi, ctx, thread + ctx->hdlr->offsets[KTHREAD__PROCESS]);
    }
done:
    return process;
}

addr_t hfm_fileobj_from_handle(vmi_instance_t vmi, context_t *ctx, reg_t handle)
{
    addr_t file_obj = 0;
    addr_t handle_obj = 0;
    addr_t process = hfm_get_current_process(vmi, ctx);
    if (!process) goto done;
    addr_t handletable = hfm_read_addr(vmi, ctx, process + ctx->hdlr->offsets[EPROCESS__OBJECT_TABLE]);
    if (!handletable) goto done;
    addr_t tablecode = hfm_read_addr(vmi, ctx, handletable + ctx->hdlr->offsets[HANDLE_TABLE__TABLE_CODE]);
    if (!tablecode) goto done;
    uint32_t handlecount = hfm_read_32(vmi, ctx,handletable + ctx->hdlr->offsets[HANDLE_TABLE__HANDLE_COUNT]);
    if (!handlecount) goto done;
    addr_t table_base = tablecode & ~EX_FAST_REF_MASK;
    uint32_t table_levels = tablecode & EX_FAST_REF_MASK;

    reg_t handle_idx = handle / HANDLE_MULTIPLIER;
    addr_t object_offset = 0;
    if (ctx->hdlr->winver == VMI_OS_WINDOWS_8) {
        addr_t object_offset = ctx->hdlr->offsets[HANDLE_TABLE_ENTRY__OBJECT_POINTER_BITS];
    }
    else {
        addr_t object_offset = ctx->hdlr->offsets[HANDLE_TABLE_ENTRY__OBJECT];
    }
    switch (table_levels) {
        case 0:
            handle_obj = hfm_read_addr(vmi, ctx, table_base + handle_idx * ctx->hdlr->sizes[HANDLE_TABLE_ENTRY] + object_offset);
            break;
        case 1:
        {
            addr_t table = 0;
            size_t psize = (ctx->hdlr->pm == VMI_PM_IA32E ? 8 : 4);
            uint32_t lowest_count = VMI_PS_4KB / ctx->hdlr->sizes[HANDLE_TABLE_ENTRY];   //Number of handle entry in the lowest level table
            uint32_t i = handle_idx % lowest_count;
            handle_idx -= i;
            uint32_t j = handle_idx / lowest_count;
            table = hfm_read_addr(vmi, ctx, table_base + j * psize);
            if (table) {
                handle_obj = hfm_read_addr(vmi, ctx, table + i * ctx->hdlr->sizes[HANDLE_TABLE_ENTRY] + object_offset);
            }
            break;
        }
        case 2:
        {
            addr_t table = 0, table2 = 0;
            size_t psize = (ctx->hdlr->pm == VMI_PM_IA32E ? 8 : 4);
            uint32_t lowest_count = VMI_PS_4KB / ctx->hdlr->sizes[HANDLE_TABLE_ENTRY];
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
                handle_obj = hfm_read_addr(vmi, ctx, table2 + i * ctx->hdlr->sizes[HANDLE_TABLE_ENTRY] + object_offset);
            break;
        }
    }
    switch (ctx->hdlr->winver) {
        case VMI_OS_WINDOWS_7:
        case VMI_OS_WINDOWS_VISTA:
            handle_obj &= ~EX_FAST_REF_MASK;
            break;
        case VMI_OS_WINDOWS_8:
            if (ctx->hdlr->pm == VMI_PM_IA32E)
                handle_obj = (((handle_obj  & VMI_BIT_MASK(19,63)) >> 20) << 4) | 0xFFFF000000000000;
            else
                handle_obj &= VMI_BIT_MASK(2,31);
            break;
        default:
        case VMI_OS_WINDOWS_10:
            if (ctx->hdlr->pm == VMI_PM_IA32E)
                handle_obj = ((handle_obj & VMI_BIT_MASK(19,63)) >> 16) | 0xFFFF000000000000;
            else
                handle_obj &= VMI_BIT_MASK(2,31);
            break;
    }
    file_obj = handle_obj + ctx->hdlr->offsets[OBJECT_HEADER__BODY];
done:
    return file_obj;
}

int hfm_read_filename_from_object(vmi_instance_t vmi, context_t *ctx, addr_t file_object, char *filename)
{
    int filepath_len = 0;
    int drivename_len = 0;

    /* Find the drive label (device name) from file object */
    char drivename[STR_BUFF] = "";
    addr_t device_object = hfm_read_addr(vmi, ctx, file_object + ctx->hdlr->offsets[FILE_OBJECT__DEVICE_OBJECT]);
    addr_t device_header = device_object - ctx->hdlr->offsets[OBJECT_HEADER__BODY];
    addr_t device_name_info_offset = 0;

    device_name_info_offset += ctx->hdlr->sizes[OBJECT_HEADER_NAME_INFO];
    addr_t device_name_info_addr = device_header - device_name_info_offset;
    hfm_read_unicode(vmi, ctx, device_name_info_addr + ctx->hdlr->offsets[OBJECT_HEADER_NAME_INFO__NAME], drivename);
    drivename_len = strlen(drivename);
    GSList *it = NULL;
    for (it = ctx->hdlr->drives; it; it = it->next) {
        drive_t *drive = (drive_t *)it->data;
        if (!strncmp(drivename, drive->win_name, STR_BUFF)) {
            sprintf(drivename, "%s", drive->dos_name);
            drivename_len = strlen(drive->dos_name);
            break;
        }
    }

    /* Find filepath from file object */
    char filepath[STR_BUFF] = "";
    filepath_len = hfm_read_unicode(vmi, ctx, file_object + ctx->hdlr->offsets[FILE_OBJECT__FILE_NAME], filepath);

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
    uint16_t length = hfm_read_16(vmi, ctx, addr + ctx->hdlr->offsets[UNICODE_STRING__LENGTH]);
    if (0 == length || length > VMI_PS_4KB)
        goto done;

    //Read unicode string buffer address
    addr_t buffer_addr = hfm_read_addr(vmi, ctx, addr + ctx->hdlr->offsets[UNICODE_STRING__BUFFER]);
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

int hfm_extract_file(vmi_instance_t vmi, context_t *ctx, addr_t object, char *path)
{
    int count = 0;
    addr_t sop = 0;
    addr_t datasection = 0, sharedcachemap = 0, imagesection = 0;
    sop = hfm_read_addr(vmi, ctx, object + ctx->hdlr->offsets[FILE_OBJECT__SECTION_OBJECT_POINTER]);
    datasection = hfm_read_addr(vmi, ctx, sop + ctx->hdlr->offsets[SECTION_OBJECT_POINTERS__DATA_SECTION_OBJECT]);
    if (datasection) {
        if (!_extract_ca_file(vmi, ctx, datasection, path))
            count++;
    }
    sharedcachemap = hfm_read_addr(vmi, ctx, sop + ctx->hdlr->offsets[SECTION_OBJECT_POINTERS__SHARED_CACHE_MAP]);
    //TODO: extraction from sharedcachedmap
    imagesection = hfm_read_addr(vmi, ctx, sop + ctx->hdlr->offsets[SECTION_OBJECT_POINTERS__IMAGE_SECTION_OBJECT]);
    if (imagesection && imagesection != datasection) {
        if (!_extract_ca_file(vmi, ctx, imagesection, path))
            count++;
    }
    return count;
}

vmi_pid_t hfm_get_process_pid(vmi_instance_t vmi, context_t *ctx)
{
    vmi_pid_t pid;
    if (!ctx->process_base) {
        ctx->process_base = hfm_get_current_process(vmi, ctx);
    }
    ctx->access_ctx.addr = ctx->process_base + ctx->hdlr->offsets[EPROCESS__UNIQUE_PROCESS_ID];
    if (VMI_SUCCESS != vmi_read_32(vmi, &ctx->access_ctx, &pid)) {
        writelog(LV_ERROR, "Failed to get the pid of process %p", ctx->process_base);
        pid = -1;
    }
    return pid;
}

static int _extract_ca_file(vmi_instance_t vmi, context_t *ctx, addr_t control_area, char *path)
{
    addr_t subsection = control_area + ctx->hdlr->sizes[CONTROL_AREA];
    addr_t segment = 0, test = 0, test2 = 0;

    uint8_t mmpte_size;
    if (VMI_PM_LEGACY == ctx->hdlr->pm)
        mmpte_size = 4;
    else
        mmpte_size = 8;
    /* Check whether subsection points back to the control area */
    segment = hfm_read_addr(vmi, ctx, control_area + ctx->hdlr->offsets[CONTROL_AREA__SEGMENT]);
    test = hfm_read_addr(vmi, ctx, segment + ctx->hdlr->offsets[SEGMENT__CONTROL_AREA]);
    if (test != control_area)
        return -1;
    test = hfm_read_64(vmi, ctx, segment + ctx->hdlr->offsets[SEGMENT__SIZE_OF_SEGMENT]);
    test2 = hfm_read_32(vmi, ctx, segment + ctx->hdlr->offsets[SEGMENT__TOTAL_NUMBER_OF_PTES]);
    if (test != (test2 * 4096))
        return -1;
    FILE *fp = fopen(path, "w");
    if (!fp) {
        writelog(LV_ERROR, "Cannot create new file %s. Please check extract_base in config file again", path);
        return -1;
    }
    while (subsection)
    {
        /* Check whether subsection points back to the control area */
        test = hfm_read_addr(vmi, ctx, subsection + ctx->hdlr->offsets[SUBSECTION__CONTROL_AREA]);
        if (test != control_area)
            break;
        addr_t base = 0, start = 0;
        uint32_t ptes = 0;
        base = hfm_read_addr(vmi, ctx, subsection + ctx->hdlr->offsets[SUBSECTION__SUBSECTION_BASE]);
        if (!(base & VMI_BIT_MASK(0,11)))
            break;
        ptes = hfm_read_32(vmi, ctx, subsection + ctx->hdlr->offsets[SUBSECTION__PTES_IN_SUBSECTION]);
        if (ptes == 0)
            break;
        start = hfm_read_32(vmi, ctx, subsection + ctx->hdlr->offsets[SUBSECTION__STARTING_SECTOR]);
        /* The offset into the file is stored implicitely
           based on the PTE's location within the subsection */
        addr_t subsection_offset = start * 0x200;
        addr_t ptecount;
        for (ptecount = 0; ptecount < ptes; ptecount++) {
            addr_t pteoffset = base + mmpte_size * ptecount;
            addr_t fileoffset = subsection_offset + ptecount * 0x1000;
            addr_t pte = 0;
            if (mmpte_size != hfm_read(vmi, ctx, pteoffset, &pte, mmpte_size))
                break;
            if (ENTRY_PRESENT(1, pte)) {
                uint8_t page[4096];
                if (4096 != vmi_read_pa(vmi, VMI_BIT_MASK(12,48) & pte, page, 4096))
                    continue;
                if (!fseek(fp, fileoffset, SEEK_SET))
                    fwrite(page, 4096, 1, fp);
            }
        }
        subsection = hfm_read_addr(vmi, ctx, subsection + ctx->hdlr->offsets[SUBSECTION__NEXT_SUBSECTION]);
    }
    fclose(fp);
}
