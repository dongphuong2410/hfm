/**
  * @file strace.h
  * @author phuong.do
  * @date 2017-06-08
  * @brief Provide services related to LibVMI
  */
#ifndef __STRACE_H__
#define __STRACE_H__

#include "private.h"

hfm_status_t strace_register(vmhdlr_t *handler, const char *func_name);
void strace_destroy(vmhdlr_t *handler);

#endif
