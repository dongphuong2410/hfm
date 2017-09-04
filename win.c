#include <stdlib.h>

#include "log.h"
#include "win.h"
#include "constants.h"
#include "win_offsets.h"
#include "rekall.h"

addr_t _adjust_obj_addr(win_ver_t winver, page_mode_t pm, addr_t obj);

void win_fill_offsets(const char *rekall_profile, addr_t *offsets)
{
    int i;
    for (i = 0; i < REKALL_OFFSETS_MAX; i++) {
        rekall_lookup(rekall_profile, win_offset_names[i][0], win_offset_names[i][1], offsets + i, NULL);
    }
    //TODO
    offsets[FILE_RENAME_INFORMATION__FILE_NAME_LENGTH] = 16;
    offsets[FILE_RENAME_INFORMATION__FILE_NAME] = 20;
    offsets[FILE_DISPOSITION_INFORMATION__DELETE_FILE] = 0;
}

void win_fill_sizes(const char *rekall_profile, addr_t *sizes)
{
    int i;
    for (i = 0; i < WIN_SIZES_MAX; i++) {
        rekall_lookup(rekall_profile, win_size_names[i], NULL, NULL, sizes + i);
    }
}

/**
  * Drives information is in /GLOBAL?? directory
  * In Windows 7, that directory is handled by System process
  * In Windows Vista, that directory is handled by smss.exe process
  */
GSList *win_list_drives(vmhdlr_t *hdlr)
{
    GSList *list = NULL;
    pid_t pid = 4;
    vmi_instance_t vmi = hdlr->vmi;
    win_ver_t winver = vmi_get_winver(vmi);
    page_mode_t pm = vmi_get_page_mode(vmi, 0);
    uint32_t psize = (pm == VMI_PM_IA32E ? 8 : 4);

    addr_t process_addr = 0;
    char *process_name = "";
    if (hdlr->winver == VMI_OS_WINDOWS_VISTA
            || hdlr->winver == VMI_OS_WINDOWS_XP) {
        process_name = "smss.exe";
    }
    else {
        process_name = "System";
    }
    GSList *processes = win_cur_processes(hdlr);
    while (processes) {
        process_t *p = (process_t *)processes->data;
        if (0 == strncmp(p->name, process_name, STR_BUFF)) {
            process_addr = p->addr;
            break;
        }
        processes = processes->next;
    }
    g_slist_free_full(processes, free);

    addr_t handle_table = 0;
    vmi_read_addr_va(vmi, process_addr + hdlr->offsets[EPROCESS__OBJECT_TABLE], pid, &handle_table);

    addr_t tablecode;
    if (VMI_FAILURE == vmi_read_addr_va(vmi, handle_table + hdlr->offsets[HANDLE_TABLE__TABLE_CODE], pid, &tablecode)) {
        writelog(LV_ERROR, "Failed to read tablecode");
        goto done;
    }

    GSList *objs = NULL;
    addr_t table_base = tablecode & ~EX_FAST_REF_MASK;
    uint32_t table_levels = tablecode & EX_FAST_REF_MASK;
    switch (table_levels) {
        case 0:
        {
            int i;
            uint32_t lowest_count = VMI_PS_4KB / hdlr->sizes[HANDLE_TABLE_ENTRY];
            for (i = 0; i < lowest_count; i++) {
                addr_t obj = 0;
                vmi_read_addr_va(vmi, table_base + i * hdlr->sizes[HANDLE_TABLE_ENTRY] + hdlr->offsets[HANDLE_TABLE_ENTRY__OBJECT], pid, &obj);
                if (obj) {
                    objs = g_slist_append(objs, (gpointer)_adjust_obj_addr(winver, pm, obj));
                }
            }
            break;
        }
        case 1:
        {
            addr_t table = 0;
            uint32_t lowest_count = VMI_PS_4KB / hdlr->sizes[HANDLE_TABLE_ENTRY];
            uint32_t table_no = VMI_PS_4KB / psize;
            int i, j;
            for (i = 0; i < table_no; i++) {
                vmi_read_addr_va(vmi, table_base + i * psize, pid, &table);
                if (table) {
                    for (j = 0; j < lowest_count; j++) {
                        addr_t entry = table + j * hdlr->sizes[HANDLE_TABLE_ENTRY];
                        addr_t obj = 0;
                        vmi_read_addr_va(vmi, entry + hdlr->offsets[HANDLE_TABLE_ENTRY__OBJECT], pid, &obj);
                        if (obj) {
                            objs = g_slist_append(objs, (gpointer)_adjust_obj_addr(winver, pm, obj));
                        }
                    }
                }
            }
            break;
        }
        case 2:
        {
            addr_t table = 0, table2 = 0;
            size_t psize = (pm == VMI_PM_IA32E ? 8 : 4);
            uint32_t low_count = VMI_PS_4KB / hdlr->sizes[HANDLE_TABLE_ENTRY];
            uint32_t mid_count = VMI_PS_4KB / psize;
            uint32_t i, j , k;
            uint32_t table_no = VMI_PS_4KB / psize;
            for (i = 0; i < table_no; i++) {
                vmi_read_addr_va(vmi, table_base + i * psize, pid, &table);
                if (table) {
                    for (j = 0; j < mid_count; j++) {
                        vmi_read_addr_va(vmi, table + j * psize, pid, &table2);
                        if (table2) {
                            for (k = 0; k < low_count; k++) {
                                addr_t entry = table2 + k * hdlr->sizes[HANDLE_TABLE_ENTRY];
                                addr_t obj = 0;
                                vmi_read_addr_va(vmi, entry + hdlr->offsets[HANDLE_TABLE_ENTRY__OBJECT], pid, &obj);
                                if (obj) {
                                    objs = g_slist_append(objs, (gpointer)_adjust_obj_addr(winver, pm, obj));
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
    }
    addr_t global_dir = 0;
    if (objs) {
        GSList *iterator = NULL;
        for (iterator = objs; iterator; iterator = iterator->next) {
            addr_t obj = (addr_t)iterator->data;
            uint8_t object_type = win_get_object_type(hdlr, pid, obj);
            if (object_type == OBJECT_TYPE_DIRECTORY) {
                addr_t name_info = obj - hdlr->sizes[OBJECT_HEADER_NAME_INFO];
                addr_t dirname = name_info + hdlr->offsets[OBJECT_HEADER_NAME_INFO__NAME];
                unicode_string_t *us = vmi_read_unicode_str_va(vmi, dirname, pid);
                unicode_string_t out = { .contents = NULL };
                if (us) {
                    status_t status = vmi_convert_str_encoding(us, &out, "UTF-8");
                    if (VMI_SUCCESS == status) {
                        if (0 == strncmp(out.contents, "GLOBAL??", 8)) {
                            global_dir = obj + hdlr->offsets[OBJECT_HEADER__BODY];
                        }
                        g_free(out.contents);
                    }
                    vmi_free_unicode_str(us);
                }
                if (global_dir) {
                    break;
                }
            }
        }
    }

    if (global_dir) {
        addr_t bucket_addr = global_dir + hdlr->offsets[OBJECT_DIRECTORY__HASH_BUCKETS];
        uint32_t bucket_size = hdlr->offsets[OBJECT_DIRECTORY__LOCK] / psize;
        int i;
        addr_t dir_entry;
        for (i = 0; i < bucket_size; i++) {
            dir_entry = 0;
            vmi_read_addr_va(vmi, bucket_addr + psize * i, pid, &dir_entry);
            while (dir_entry) {
                addr_t obj = 0;
                vmi_read_addr_va(vmi, dir_entry + hdlr->offsets[OBJECT_DIRECTORY_ENTRY__OBJECT], pid, &obj);
                if (obj) {
                    //Check type of object,for SymbolicLink object
                    addr_t obj_header = obj - hdlr->offsets[OBJECT_HEADER__BODY];
                    uint8_t type_index = win_get_object_type(hdlr, pid, obj_header);
                    if (type_index == OBJECT_TYPE_SYMBOLIC_LINK) {
                        uint32_t drive_index = 0;
                        vmi_read_32_va(vmi, obj + hdlr->offsets[OBJECT_SYMBOLIC_LINK__DOS_DEVICE_DRIVE_INDEX], pid, &drive_index);
                        if (drive_index > 0) {
                            drive_t *drive = (drive_t *)calloc(1, sizeof(drive_t));
                            drive->index = drive_index;
                            sprintf(drive->dos_name, "%c:", 'A' + drive_index - 1);
                            unicode_string_t *us = vmi_read_unicode_str_va(vmi, obj + hdlr->offsets[OBJECT_SYMBOLIC_LINK__LINK_TARGET], pid);
                            unicode_string_t out = { .contents = NULL };
                            if (us) {
                                status_t status = vmi_convert_str_encoding(us, &out, "UTF-8");
                                if (VMI_SUCCESS == status) {
                                    int len = strlen("\\Device\\");
                                    //Remove \Device part if exists
                                    if (!strncmp(out.contents, "\\Device\\", len))
                                        sprintf(drive->win_name, "%s", out.contents + len);
                                    else
                                        sprintf(drive->win_name, "%s", out.contents);
                                    g_free(out.contents);
                                }
                                vmi_free_unicode_str(us);
                            }
                            list = g_slist_append(list, drive);
                        }
                    }
                }
                vmi_read_addr_va(vmi, dir_entry + hdlr->offsets[OBJECT_DIRECTORY_ENTRY__CHAIN_LINK], pid, &dir_entry);
            }
        }
    }
done:
    return list;
}

GSList *win_cur_processes(vmhdlr_t *hdlr)
{
    vmi_instance_t vmi = hdlr->vmi;
    GSList *list = NULL;
    addr_t list_head = 0, next_list_entry = 0;
    addr_t current_process = 0;
    if (VMI_FAILURE == vmi_read_addr_ksym(vmi, "PsActiveProcessHead", &list_head)) {
        writelog(LV_ERROR, "Failed to find PsActiveProcessHead");
        goto done;
    }
    next_list_entry = list_head;
    /* Walk the task list */
    do {
        current_process = next_list_entry - hdlr->offsets[EPROCESS__ACTIVE_PROCESS_LINKS];
        if (VMI_FAILURE == vmi_read_addr_va(vmi, next_list_entry, 0, &next_list_entry)) {
            writelog(LV_ERROR, "Failed to read next pointer in loop");
            break;
        }
        if (next_list_entry == list_head) {
            break;
        }
        pid_t pid = 0;
        if (VMI_SUCCESS == vmi_read_32_va(vmi, current_process + hdlr->offsets[EPROCESS__UNIQUE_PROCESS_ID], 0, &pid)) {
            process_t *process = (process_t *)calloc(1, sizeof(process_t));
            process->pid = pid;
            process->addr = current_process;
            char *str = vmi_read_str_va(vmi, current_process + hdlr->offsets[EPROCESS__IMAGE_FILE_NAME], 0);
            if (str) {
                strncpy(process->name, str, STR_BUFF);
                free(str);
            }
            list = g_slist_append(list, process);
        }
    } while (1);
done:
    return list;
}

addr_t _adjust_obj_addr(win_ver_t winver, page_mode_t pm, addr_t obj)
{
    addr_t addr = obj;
    switch (winver) {
        case VMI_OS_WINDOWS_7:
        case VMI_OS_WINDOWS_VISTA:
            addr &= (~EX_FAST_REF_MASK);
            break;
        case VMI_OS_WINDOWS_8:
            if (pm == VMI_PM_IA32E)
                addr = ((addr & VMI_BIT_MASK(19,63)) >> 16) | 0xFFFFE00000000000;
            else
                addr = addr & VMI_BIT_MASK(2,31);
        default:
        case VMI_OS_WINDOWS_10:
            if (pm == VMI_PM_IA32E)
                addr = ((addr & VMI_BIT_MASK(19,63)) >> 16) | 0xFFFF000000000000;
            else
                addr = addr & VMI_BIT_MASK(2,31);
    }
    return addr;
}

object_t win_get_object_type(vmhdlr_t *hdlr, pid_t pid, addr_t object_header)
{
    uint8_t type_index = 0;
    if (VMI_OS_WINDOWS_7 == hdlr->winver) {
        if (VMI_SUCCESS != vmi_read_8_va(hdlr->vmi, object_header + hdlr->offsets[OBJECT_HEADER__TYPE_INDEX], pid, &type_index)) {
            writelog(LV_ERROR, "Error read object type");
            goto done;
        }
        if (type_index == 0x3)
            return OBJECT_TYPE_DIRECTORY;
        else if (type_index == 0x4)
            return OBJECT_TYPE_SYMBOLIC_LINK;
    }
    else if (VMI_OS_WINDOWS_VISTA == hdlr->winver
            || VMI_OS_WINDOWS_XP == hdlr->winver) {
        addr_t type = 0;
        if (VMI_SUCCESS != vmi_read_addr_va(hdlr->vmi, object_header + hdlr->offsets[OBJECT_HEADER__TYPE], pid, &type)) {
            writelog(LV_ERROR, "Error read object type");
            goto done;
        }
        if (VMI_SUCCESS != vmi_read_8_va(hdlr->vmi, type + hdlr->offsets[OBJECT_TYPE__INDEX], pid, &type_index)) {
            writelog(LV_ERROR, "Error read object type index");
            goto done;
        }
        if (type_index == 0x2)
            return OBJECT_TYPE_DIRECTORY;
        else if (type_index == 0x3)
            return OBJECT_TYPE_SYMBOLIC_LINK;
    }
done:
    return OBJECT_TYPE_UNKNOWN;
}
