/**
  * @file attr_changed.h
  * @author phuong.do
  * @date 2017-07-27
  * @brief Atrr_changed monitor plugin
  */

#ifndef __HFM_MONITOR_ATTR_CHANGED_H__
#define __HFM_MONITOR_ATTR_CHANGED_H__

#include "private.h"
#include "policy.h"

/**
  * @brief Implement of abstract function mon_add_policy for attr_changed plugin_
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t attr_changed_add_policy(vmhdlr_t *hdlr, policy_t *policy);

#endif
