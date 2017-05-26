/**
  * @file owner_changed.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Permission_changed monitor plugin
  */

#ifndef __HFM_MONITOR_OWNER_CHANGED_H__
#define __HFM_MONITOR_OWNER_CHANGED_H__

#include "private.h"

/**
  * @brief Implement of abstract function mon_add_policy for owner_changed plugin
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t owner_changed_add_policy(vmhdlr_t *hdlr, policy_t *policy);

#endif
