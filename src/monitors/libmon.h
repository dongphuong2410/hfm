/**
  * @file libmon.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Abstract file for the plugins
  */

#ifndef __HFM_LIBMON_H__
#define __HFM_LIBMON_H__

#include "private.h"

/**
  * @brief Init monitors
  *
  * @param type monitor type
  */
int mon_init(void);

/**
  * @brief Add a policy to the vm
  *
  * @param hdlr Pointer to vm
  * @param policy Pointer to a policy
  * @return FAIL or SUCCESS
  */
hfm_status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy);

/**
  * @brief Close monitor
  *
  */
void mon_close(void);
#endif
