/**
  * @file vmi_helper.h * @author phuong.do
  * @date 2017-05-24
  * @brief Provide services related to LibVMI
  */
#ifndef __HFM_VMI_HELPER_H__
#define __HFM_VMI_HELPER_H__

#include "private.h"

/**
  * @brief Init a vm handler
  *
  * @param handler Pointer to vmhdlr_t
  * @return FAIL or SUCCESS
  */
hfm_status_t vh_init(vmhdlr_t *handler);

/**
  * @brief Listen to an event on a vm
  *
  * @param handler Pointer to vmhdlr_t
  */
void vh_listen(vmhdlr_t *handler);

/**
  * @brief Monitor a syscall
  *
  * @param handler Pointer to handler
  * @param name Syscall name
  * @param pre_cb Callback function that will be called at the beginning of the syscall
  * @param post_cb Callback function that will be called at the end of the syscall
    * @return FAIL or SUCCESS
    */
hfm_status_t vh_monitor_syscall(vmhdlr_t *handler, const char *name, void *pre_cb, void *post_cb);

/**
  * @brief Close the vm handler
  * @param handler Pointer to vmhdlr_t
  */
void vh_close(vmhdlr_t *handler);

/**
  * @brief Inject a trap to the machine
  * @param handler Pointer to vmhdlr_t
  * @param va Virtual address
  */
void vh_inject_trap(vmhdlr_t *handler, addr_t va);

/**
  * @brief Remove a trap from machine memory
  */
void vh_delete_trap(vmhdlr_t *handler);
#endif
