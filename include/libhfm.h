/**
  * @file libhfm.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Provide services related to LibVMI
  */
#ifndef __LIBHFM_H__
#define __LIBHFM_H__

#include "private.h"

/**
  * @brief Init a vm handler
  *
  * @param handler Pointer to vmhdlr_t
  * @return FAIL or SUCCESS
  */
hfm_status_t hfm_init(vmhdlr_t *handler);

/**
  * @brief Listen to an event on a vm
  *
  * @param handler Pointer to vmhdlr_t
  */
void hfm_listen(vmhdlr_t *handler);

/**
  * @brief Close the vm handler
  * @param handler Pointer to vmhdlr_t
  */
void hfm_close(vmhdlr_t *handler);

/**
  * @brief Register a trap for syscall
  */
hfm_status_t hfm_monitor_syscall(vmhdlr_t *handler, const char *func_name, event_response_t (*cb)(vmhdlr_t *, trap_data_t *));

/**
  * @brief Clean all the trap
  */
void hfm_destroy_traps(vmhdlr_t *handler);

#endif
