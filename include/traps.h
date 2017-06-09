/**
  * @file traps.h
  * @author phuong.do
  * @date 2017-06-08
  * @brief Provide services related to LibVMI
  */
#ifndef __TRAPS_H__
#define __TRAPS_H__

#include "private.h"

hfm_status_t traps_register(vmhdlr_t *handler, const char *func_name);
void traps_destroy(vmhdlr_t *handler);

#endif
