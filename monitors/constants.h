/**
  * @file constants.h
  * @author phuong.do
  * @date 2017-06-28
  * @brief Define constants
  */

#ifndef __HFM_MONITOR_CONSTANTS_H__
#define __HFM_MONITOR_CONSTANTS_H__

#define STATUS_SUCCESS 0

enum {
    FILE_CREATED = 0,
    FILE_OPENED,
    FILE_OVERWRITTEN,
    FILE_SUPERSEDED,
    FILE_EXISTS,
    FILE_DOES_NOT_EXIST
};

#endif
