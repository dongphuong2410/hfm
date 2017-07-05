/**
  * @file constants.h
  * @author phuong.do
  * @date 2017-06-28
  * @brief Define constants
  */

#ifndef __HFM_MONITOR_CONSTANTS_H__
#define __HFM_MONITOR_CONSTANTS_H__

#define STATUS_SUCCESS 0

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

addr_t OBJEC_ATTRIBUTES_OBJECT_NAME;
addr_t UNICODE_STRING_LENGTH;
addr_t UNICODE_STRING_BUFFER;
addr_t IO_STATUS_BLOCK_INFORMATION;
addr_t IO_STATUS_BLOCK_STATUS;
addr_t FILE_RENAME_INFORMATION_FILE_NAME_LENGTH;
addr_t FILE_RENAME_INFORMATION_FILE_NAME;

static int constants_init(const char *rekall_profile)
{
    int status = 0;
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "ObjectName", &OBJEC_ATTRIBUTES_OBJECT_NAME, NULL);
    status |= rekall_lookup(rekall_profile, "_OBJECT_ATTRIBUTES", "Length", &UNICODE_STRING_LENGTH, NULL);
    status |= rekall_lookup(rekall_profile, "_UNICODE_STRING", "Buffer", &UNICODE_STRING_BUFFER, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Information", &IO_STATUS_BLOCK_INFORMATION, NULL);
    status |= rekall_lookup(rekall_profile, "_IO_STATUS_BLOCK", "Status", &IO_STATUS_BLOCK_STATUS, NULL);
    FILE_RENAME_INFORMATION_FILE_NAME_LENGTH = 16; //TODO : value calculated by debug, just confirmed for Windows 7 only
    FILE_RENAME_INFORMATION_FILE_NAME = 20; //TODO
    return status;
}

#endif
