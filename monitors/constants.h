/**
  * @file constants.h
  * @author phuong.do
  * @date 2017-06-28
  * @brief Define constants
  */

#ifndef __HFM_MONITOR_CONSTANTS_H__
#define __HFM_MONITOR_CONSTANTS_H__


/* Define the I/O status information return values for NtCreateFile/NtOpenFile */
#define FILE_SUPERSEDED         0x00000000
#define FILE_OPENED             0x00000001
#define FILE_CREATED            0x00000002
#define FILE_OVERWRITTEN        0x00000003
#define FILE_EXISTS             0x00000004
#define FILE_DOES_NOT_EXIST     0x00000005

#endif
