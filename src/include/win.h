#ifndef __HFM_VMI_HELPER_H
#define __HFM_VMI_HELPER_H

#include <libvmi/libvmi.h>
#include <glib.h>
#include "private.h"
#include "constants.h"

typedef struct _drive {
    char dos_name[1024];
    char win_name[1024];
    uint32_t index;
} drive_t;

typedef struct _process {
    char name[1024];
    uint32_t pid;
    addr_t addr;
} process_t;

/**
  * @brief Fill the offsets of kernel structs
  * @param[in] rekall_profile Rekall profile path
  * @param[out] offsets Array to keep the offsets output
  */
void win_fill_offsets(const char *rekall_profile, addr_t *offsets);

/**
  * @brief Fill the sizes of kernel structs
  * @param[in] rekall_profile Rekall profile path
  * @param[out] sizes Array to keep the sizes output
  */
void win_fill_sizes(const char *rekall_profile, addr_t *sizes);

/**
  * @brief Return list of current processes
  * @param[in] hdlr vmhdlr_t
  * @return list of current processes
  */
GSList *win_cur_processes(vmhdlr_t *hdlr);

/**
  * @brief Read ObHeaderCookie
  * @param[in] hdlr vmhdlr_t
  * @return ObHeaderCookie
  */
uint8_t win_ob_header_cookie(vmhdlr_t *hdlr);

/**
  * @brief Get the object type from object_header
  * @param[in] hdlr vmhdlr_t
  * @param[in] pid Pid
  * @param[in] object_header Address of _OBJECT_HEADER struct
  * @return object type
  */
object_t win_get_object_type(vmhdlr_t *hdlr, pid_t pid, addr_t object_header);

/**
  * @brief List all harddrive names in the OS
  * @param[in] hdlr vmhdlr
  * @return List of hard drives
  */
GSList *win_list_drives(vmhdlr_t *hdlr);

#endif
