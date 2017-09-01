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

typedef struct _xen_interface {
    xc_interface *xc;
    libxl_ctx *xl_ctx;
    domid_t domID;
} xen_interface_t;

xen_interface_t *xen_init_interface(const char *name);
void xen_free_interface(xen_interface_t *xen);
/**
  * @brief Extend the vm memory to proposed_memsize
  * @param xen Pointer to xen
  * @param proposed_memsize Size of memory that want to extend to
  * @return Address of the memory frame that has been added
  */
addr_t xen_alloc_shadow_frame(xen_interface_t *xen, uint64_t proposed_memsize);

/**
  * @brief Free the allocated frame
  * @param xen Pointer to xen_interface_t
  * @param frame Address of allocated frame
  */
void xen_free_shadow_frame(xen_interface_t *xen, uint64_t *frame);

#endif
