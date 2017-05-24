/**
  * @file hidden_changed.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Hidden_changed monitor plugin
  */

#ifndef __HFM_MONITOR_HIDDEN_CHANGED_H__
#define __HFM_MONITOR_HIDDEN_CHANGED_H__

/**
  * @brief Implement of abstract function mon_add_policy for hidden_changed plugin
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
status_t hidden_changed_add_policy(vmhdlr_t *hdlr, policy_t *policy);

#endif
