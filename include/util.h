/**
  * @file util.h
  * @author phuong.do
  * @date 2017-06-21
  * @brief Utilities
  */
#ifndef __HFM_UTIL_H__
#define __HFM_UTIL_H__

#include <stdint.h>
#include <libvmi/libvmi.h>

/**
  * @brief Search the kernel code address space for an existing int 3 instruction
  * @param vmi vmi instance
  * @return trampoline address
  */
uint64_t util_find_trampoline_addr(vmi_instance_t vmi);

/**
  * @brief Return the return address of the function that call the syscall
  * @param vmi vmi instance
  * @return return address
  */
uint64_t util_find_return_addr(vmi_instance_t vmi);

#endif
