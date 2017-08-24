#ifndef __HFM_VMI_HELPER_H
#define __HFM_VMI_HELPER_H

#include <libvmi/libvmi.h>
#include <glib.h>

typedef struct _drive {
    char dos_name[1024];
    char win_name[1024];
    uint32_t index;
} drive_t;

/**
  * @brief Fill the offsets of kernel structs
  * @param[in] vmi vmi_instance_t
  * @param[out] offsets Array to keep the offsets output
  */
void win_fill_offsets(vmi_instance_t vmi, addr_t *offsets);

/**
  * @brief Get address of a process from its pid
  * @param[in] vmi vmi_instance_t
  * @param[in] pid Process Id
  * @return address of _EPROCESS struct, return 0x0 if failed
  */
addr_t win_get_process(vmi_instance_t vmi, pid_t pid);

/**
  * @brief List all harddrive names in the OS
  * @param[in] vmi vmi_instance_t
  * @return List of hard drives
  */
GSList *win_list_drives(vmi_instance_t vmi);

#endif
