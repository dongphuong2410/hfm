/**
  * @file hfm.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Main logic of the program, managing a VM 
  */
#ifndef __HFM_HFM_H__
#define __HFM_HFM_H__

#include "private.h"

/**
  * brief Init vmhdlr for a VM
  *
  * @param vm VM name
  * @return Pointer to vmhdlr_t
  */
vmhdlr_t *hfm_init(char *vm);

/**
  * brief Apply policies for the VM
  *
  * @param vm Pointer to vmhdlr_t
  * @param policies List of policy
  * @return Succeed or fail
  */
status_t hfm_set_policies(vmhdlr_t *vm, GSList *policies);

/**
  * brief Start monitoring the vm
  *
  * @param vm Pointer to vmhdlr_t
  * @return Succeed or fail
  */
status_t hfm_run(vmhdlr_t *vm);

/**
  * brief Close the vm
  * @param vm Pointer to vmhdlr_t
  */
void hfm_close(vmhdlr_t *vm);

#endif


