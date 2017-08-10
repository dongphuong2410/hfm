/**
  * @file constants.h
  * @author phuong.do
  * @date 2017-06-28
  * @brief Define constants
  */

#ifndef __HFM_MONITOR_CONSTANTS_H__
#define __HFM_MONITOR_CONSTANTS_H__

#define STATUS_SUCCESS 0
#define EX_FAST_REF_MASK 7
#define HANDLE_MULTIPLIER 4
#define BIT32 0
#define BIT64 1
#define PM2BIT(pm) ((pm == VMI_PM_IA32E) ? BIT64 : BIT32)
#define WIN7_TYPEINDEX_LAST 44
#define NT_SUCCESS(status)  ((status) >= 0 && (status) <= 0x7FFFFFFF)

#define OB_INFOMASK_PROCESS_INFO        0x10
#define OB_INFOMASK_QUOTA               0x08
#define OB_INFOMASK_HANDLE              0x04
#define OB_INFOMASK_NAME                0x02
#define OB_INFOMASK_CREATOR_INFO        0x01

#include "rekall.h"

enum {
    FILE_CREATED = 0,
    FILE_OPENED,
    FILE_OVERWRITTEN,
    FILE_SUPERSEDED,
    FILE_EXISTS,
    FILE_DOES_NOT_EXIST
};

enum {
    FILE_DIRECTORY_INFORMATION = 1,
    FILE_FULL_DIRECTORY_INFORMATION,
    FILE_BOTH_DIRECTORY_INFORMATION,
    FILE_BASIC_INFORMATION,
    FILE_STANDARD_INFORMATION,
    FILE_INTERNAL_INFORMATION,
    FILE_EA_INFORMATION,
    FILE_ACCESS_INFORMATION,
    FILE_NAME_INFORMATION,
    FILE_RENAME_INFORMATION,
    FILE_LINK_INFORMATION,
    FILE_NAMES_INFORMATION,
    FILE_DISPOSITION_INFORMATION,
    FILE_POSITION_INFORMATION,
    FILE_FULL_EA_INFORMATION,
    FILE_MODE_INFORMATION
};

addr_t OBJECT_ATTRIBUTES_OBJECT_NAME;
addr_t OBJECT_ATTRIBUTES_ROOT_DIRECTORY;
addr_t UNICODE_STRING_LENGTH;
addr_t UNICODE_STRING_BUFFER;
addr_t IO_STATUS_BLOCK_INFORMATION;
addr_t IO_STATUS_BLOCK_STATUS;
addr_t FILE_RENAME_INFORMATION_FILE_NAME_LENGTH;
addr_t FILE_RENAME_INFORMATION_FILE_NAME;
addr_t FILE_DISPOSITION_INFORMATION_DELETE_FILE;
addr_t KPCR_PRCB;
addr_t KPCR_PRCB_DATA;
addr_t KPRCB_CURRENT_THREAD;
addr_t KTHREAD_PROCESS;
addr_t HANDLE_TABLE_HANDLE_COUNT;
addr_t HANDLE_TABLE_TABLE_CODE;
addr_t OBJECT_HEADER_BODY;
addr_t OBJECT_HEADER_INFO_MASK;
addr_t OBJECT_HEADER_TYPE_INDEX;
addr_t FILE_OBJECT_FILE_NAME;
addr_t FILE_OBJECT_SIZE;
addr_t FILE_OBJECT_DEVICE_OBJECT;
addr_t FILE_OBJECT_VPB;
addr_t VPB_VOLUME_LABEL;
addr_t VPB_VOLUME_LABEL_LENGTH;
addr_t EPROCESS_OBJECT_TABLE;
addr_t OBJECT_HEADER_NAME_INFO_SIZE;
addr_t OBJECT_HEADER_CREATOR_INFO_SIZE;
addr_t OBJECT_HEADER_NAME_INFO_NAME;
addr_t DEVICE_OBJECT_DRIVER_OBJECT;
addr_t DRIVER_OBJECT_DRIVER_NAME;
addr_t DEVICE_OBJECT_VPB;
addr_t EPROCESS_PEB;
addr_t EPROCESS_UNIQUE_PROCESS_ID;
addr_t PEB_PROCESS_PARAMETERS;
addr_t RTL_USER_PROCESS_PARAMETERS_CURRENT_DIRECTORY;
addr_t RTL_USER_PROCESS_PARAMETERS_IMAGE_PATH_NAME;
addr_t CURDIR_DOS_PATH;

size_t HANDLE_TABLE_ENTRY_SIZE;

static int constants_init(const char *rekall_profile)
{
    int status = 0;
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "ObjectName", &OBJECT_ATTRIBUTES_OBJECT_NAME, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "RootDirectory", &OBJECT_ATTRIBUTES_ROOT_DIRECTORY, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "Length", &UNICODE_STRING_LENGTH, NULL);
    status |= rekall_lookup(rekall_profile, "_UNICODE_STRING", "Buffer", &UNICODE_STRING_BUFFER, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Information", &IO_STATUS_BLOCK_INFORMATION, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Status", &IO_STATUS_BLOCK_STATUS, NULL);
    status |= rekall_lookup(rekall_profile, "_KPCR", "Prcb", &KPCR_PRCB, NULL);
    status |= rekall_lookup(rekall_profile, "_KPCR", "PrcbData", &KPCR_PRCB_DATA, NULL);
    status |= rekall_lookup(rekall_profile, "_KPRCB", "CurrentThread", &KPRCB_CURRENT_THREAD, NULL);
    status |= rekall_lookup(rekall_profile, "_KTHREAD", "Process", &KTHREAD_PROCESS, NULL);
    status |= rekall_lookup(rekall_profile, "_HANDLE_TABLE", "HandleCount", &HANDLE_TABLE_HANDLE_COUNT, NULL);
    status |= rekall_lookup(rekall_profile, "_HANDLE_TABLE", "TableCode", &HANDLE_TABLE_TABLE_CODE, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_HEADER", "Body", &OBJECT_HEADER_BODY, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_HEADER", "InfoMask", &OBJECT_HEADER_INFO_MASK, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_HEADER", "TypeIndex", &OBJECT_HEADER_TYPE_INDEX, NULL);
    status |= rekall_lookup(rekall_profile, "_FILE_OBJECT", "FileName", &FILE_OBJECT_FILE_NAME, NULL);
    status |= rekall_lookup(rekall_profile, "_FILE_OBJECT", "Size", &FILE_OBJECT_SIZE, NULL);
    status |= rekall_lookup(rekall_profile, "_FILE_OBJECT", "DeviceObject", &FILE_OBJECT_DEVICE_OBJECT, NULL);
    status |= rekall_lookup(rekall_profile, "_FILE_OBJECT", "Vpb", &FILE_OBJECT_VPB, NULL);
    status |= rekall_lookup(rekall_profile, "_VPB", "VolumeLabel", &VPB_VOLUME_LABEL, NULL);
    status |= rekall_lookup(rekall_profile, "_VPB", "VolumeLabelLength", &VPB_VOLUME_LABEL_LENGTH, NULL);
    status |= rekall_lookup(rekall_profile, "_EPROCESS", "ObjectTable", &EPROCESS_OBJECT_TABLE, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_HEADER_NAME_INFO", "Name", &OBJECT_HEADER_NAME_INFO_NAME, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_HEADER_NAME_INFO", NULL, NULL, &OBJECT_HEADER_NAME_INFO_SIZE);
    status |= rekall_lookup(rekall_profile, "_OBJECT_HEADER_CREATOR_INFO", NULL, NULL, &OBJECT_HEADER_CREATOR_INFO_SIZE);
    status |= rekall_lookup(rekall_profile, "_DEVICE_OBJECT", "DriverObject", &DEVICE_OBJECT_DRIVER_OBJECT, NULL);
    status |= rekall_lookup(rekall_profile, "_DRIVER_OBJECT", "DriverName", &DRIVER_OBJECT_DRIVER_NAME, NULL);
    status |= rekall_lookup(rekall_profile, "_DEVICE_OBJECT", "Vpb", &DEVICE_OBJECT_VPB, NULL);
    status |= rekall_lookup(rekall_profile, "_EPROCESS", "Peb", &EPROCESS_PEB, NULL);
    status |= rekall_lookup(rekall_profile, "_EPROCESS", "UniqueProcessId", &EPROCESS_UNIQUE_PROCESS_ID, NULL);
    status |= rekall_lookup(rekall_profile, "_PEB", "ProcessParameters", &PEB_PROCESS_PARAMETERS, NULL);
    status |= rekall_lookup(rekall_profile, "_RTL_USER_PROCESS_PARAMETERS", "CurrentDirectory", &RTL_USER_PROCESS_PARAMETERS_CURRENT_DIRECTORY, NULL);
    status |= rekall_lookup(rekall_profile, "_RTL_USER_PROCESS_PARAMETERS", "ImagePathName", &RTL_USER_PROCESS_PARAMETERS_IMAGE_PATH_NAME, NULL);
    status |= rekall_lookup(rekall_profile, "_CURDIR", "DosPath", &CURDIR_DOS_PATH, NULL);
    status |= rekall_lookup(rekall_profile, "_HANDLE_TABLE_ENTRY", NULL, NULL, &HANDLE_TABLE_ENTRY_SIZE);
    FILE_RENAME_INFORMATION_FILE_NAME_LENGTH = 16; //TODO : value calculated by debug, just confirmed for Windows 7 only
    FILE_RENAME_INFORMATION_FILE_NAME = 20; //TODO
    FILE_DISPOSITION_INFORMATION_DELETE_FILE = 0;

    return status;
}

#endif
