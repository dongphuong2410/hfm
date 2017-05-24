/**
  * @file monitor.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Abstract file for the plugins
  */

#ifndef __HFM_MONITOR_H__
#define __HFM_MONITOR_H__

/**
  * @brief Set the function pointers for mon_add_trap
  *
  * @param type monitor type
  */
void mon_init(monitor_t type);

/**
  * @brief Add a policy to the vm
  *
  * @param hdlr Pointer to vm
  * @param policy Pointer to a policy
  * @return FAIL or SUCCESS
  */
status_t mon_add_policy(vmhdlr_t *hdlr, policy_t *policy);

#endif
