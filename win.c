#include <stdlib.h>

#include "log.h"
#include "win.h"
#include "constants.h"

addr_t _adjust_obj_addr(win_ver_t winver, page_mode_t pm, addr_t obj);

GSList *win_list_drives(vmi_instance_t vmi)
{
    GSList *list = NULL;
    pid_t pid = 4;
    win_ver_t winver = vmi_get_winver(vmi);;
    page_mode_t pm = vmi_get_page_mode(vmi, 0);
    uint32_t psize = (pm == VMI_PM_IA32E ? 8 : 4);

    addr_t process = win_get_process(vmi, pid);
    addr_t handle_table = 0;
    vmi_read_addr_va(vmi, process + EPROCESS_OBJECT_TABLE, pid, &handle_table);

    addr_t tablecode;
    if (VMI_FAILURE == vmi_read_addr_va(vmi, handle_table + HANDLE_TABLE_TABLE_CODE, pid, &tablecode)) {
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
            uint32_t lowest_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;
            for (i = 0; i < lowest_count; i++) {
                addr_t obj = 0;
                vmi_read_addr_va(vmi, table_base + i * HANDLE_TABLE_ENTRY_SIZE + HANDLE_TABLE_ENTRY_OBJECT, pid, &obj);
                if (obj) {
                    objs = g_slist_append(objs, (gpointer)_adjust_obj_addr(winver, pm, obj));
                }
            }
            break;
        }
        case 1:
        {
            addr_t table = 0;
            uint32_t lowest_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;
            uint32_t table_no = VMI_PS_4KB / psize;
            int i, j;
            for (i = 0; i < table_no; i++) {
                vmi_read_addr_va(vmi, table_base + i * psize, pid, &table);
                if (table) {
                    for (j = 0; j < lowest_count; j++) {
                        addr_t entry = table + j * HANDLE_TABLE_ENTRY_SIZE;
                        addr_t obj = 0;
                        vmi_read_addr_va(vmi, entry + HANDLE_TABLE_ENTRY_OBJECT, pid, &obj);
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
            uint32_t low_count = VMI_PS_4KB / HANDLE_TABLE_ENTRY_SIZE;
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
                                addr_t entry = table2 + k * HANDLE_TABLE_ENTRY_SIZE;
                                addr_t obj = 0;
                                vmi_read_addr_va(vmi, entry + HANDLE_TABLE_ENTRY_OBJECT, pid, &obj);
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
            uint8_t object_type = 0;
            vmi_read_8_va(vmi, obj + OBJECT_HEADER_TYPE_INDEX, pid, &object_type);
            if (object_type == 3) {     //Directory
                addr_t name_info = obj - OBJECT_HEADER_NAME_INFO_SIZE;
                addr_t dirname = name_info + OBJECT_HEADER_NAME_INFO_NAME;
                unicode_string_t *us = vmi_read_unicode_str_va(vmi, dirname, pid);
                unicode_string_t out = { .contents = NULL };
                if (us) {
                    status_t status = vmi_convert_str_encoding(us, &out, "UTF-8");
                    if (VMI_SUCCESS == status) {
                        if (0 == strncmp(out.contents, "GLOBAL??", 8)) {
                            global_dir = obj + OBJECT_HEADER_BODY;
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
        addr_t bucket_addr = global_dir + OBJECT_DIRECTORY_HASH_BUCKETS;
        uint32_t bucket_size = OBJECT_DIRECTORY_LOCK / psize;
        int i;
        addr_t dir_entry;
        for (i = 0; i < bucket_size; i++) {
            dir_entry = 0;
            vmi_read_addr_va(vmi, bucket_addr + psize * i, pid, &dir_entry);
            while (dir_entry) {
                addr_t obj = 0;
                vmi_read_addr_va(vmi, dir_entry + OBJECT_DIRECTORY_ENTRY_OBJECT, pid, &obj);
                if (obj) {
                    //Check type of object,for SymbolicLink object
                    addr_t obj_header = obj - OBJECT_HEADER_BODY;
                    uint8_t type_index = 0;
                    vmi_read_8_va(vmi, obj_header + OBJECT_HEADER_TYPE_INDEX, pid, &type_index);
                    if (type_index == 0x4) {             //SymbolicLink
                        uint32_t drive_index = 0;
                        vmi_read_32_va(vmi, obj + OBJECT_SYMBOLIC_LINK_DOS_DEVICE_DRIVE_INDEX, pid, &drive_index);
                        if (drive_index > 0) {
                            drive_t *drive = (drive_t *)calloc(1, sizeof(drive_t));
                            drive->index = drive_index;
                            sprintf(drive->dos_name, "%c:", 'A' + drive_index - 1);
                            unicode_string_t *us = vmi_read_unicode_str_va(vmi, obj + OBJECT_SYMBOLIC_LINK_LINK_TARGET, pid);
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
                vmi_read_addr_va(vmi, dir_entry + OBJECT_DIRECTORY_ENTRY_CHAIN_LINK, pid, &dir_entry);
            }
        }
    }
done:
    return list;
}

addr_t win_get_process(vmi_instance_t vmi, pid_t pid)
{
    addr_t process_addr = 0;
    addr_t list_head = 0, next_list_entry = 0;
    addr_t current_process = 0;
    if (VMI_FAILURE == vmi_read_addr_ksym(vmi, "PsActiveProcessHead", &list_head)) {
        writelog(LV_ERROR, "Failed to find PsActiveProcessHead");
        goto done;
    }
    next_list_entry = list_head;
    /* Walk the task list */
    do {
        current_process = next_list_entry - EPROCESS_ACTIVE_PROCESS_LINKS;
        if (VMI_FAILURE == vmi_read_addr_va(vmi, next_list_entry, pid, &next_list_entry)) {
            writelog(LV_ERROR, "Failed to read next pointer in loop");
            break;
        }
        if (next_list_entry == list_head) {
            break;
        }
        pid_t cur_pid = 0;
        if (VMI_SUCCESS == vmi_read_32_va(vmi, current_process + EPROCESS_UNIQUE_PROCESS_ID, pid, &cur_pid)) {
            if (cur_pid == pid) {
                process_addr = current_process;
                break;
            }
        }
    } while (1);
done:
    return process_addr;
}

addr_t _adjust_obj_addr(win_ver_t winver, page_mode_t pm, addr_t obj)
{
    addr_t addr = obj;
    switch (winver) {
        case VMI_OS_WINDOWS_7:
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
