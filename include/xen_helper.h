/**
  * @file xen_helper.h
  * @author phuong.do
  * @date 2017-06-07
  * @brief Provide services related to XEN
  */
#ifndef __XEN_HELPER_H__
#define __XEN_HELPER_H__

#include <xenctrl.h>
#include <libxl_utils.h>
#include "private.h"

xen_interface_t *xen_init_interface(const char *name);
void xen_free_interface(xen_interface_t *xen);
int xen_enable_altp2m(xen_interface_t *xen);
int xen_create_view(xen_interface_t *xen, uint16_t *idx);
int xen_switch_view(xen_interface_t *xen, uint16_t idx);

/**
  * @brief Extend the vm memory to proposed_memsize
  * @param xen Pointer to xen
  * @param proposed_memsize Size of memory that want to extend to
  * @return Address of the memory frame that has been added
  */
addr_t xen_extend_extra_frame(xen_interface_t *xen, uint64_t proposed_memsize);

#endif
