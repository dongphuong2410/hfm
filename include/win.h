#ifndef __HFM_VMI_HELPER_H
#define __HFM_VMI_HELPER_H

#include <libvmi/libvmi.h>
#include <glib.h>
#include "private.h"

typedef struct _drive {
    char dos_name[1024];
    char win_name[1024];
    uint32_t index;
} drive_t;

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
  * @brief Get address of a process from its pid
  * @param[in] hdlr vmhdlr_t
  * @param[in] pid Process Id
  * @return address of _EPROCESS struct, return 0x0 if failed
  */
addr_t win_get_process(vmhdlr_t *hdlr, pid_t pid);

/**
  * @brief List all harddrive names in the OS
  * @param[in] hdlr vmhdlr
  * @return List of hard drives
  */
GSList *win_list_drives(vmhdlr_t *hdlr);

#endif
