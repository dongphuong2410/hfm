/**
  * @file logfile_modified.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Logfile_modified monitor plugin
  */

#ifndef __HFM_MONITOR_LOGFILE_MODIFIED_H__
#define __HFM_MONITOR_LOGFILE_MODIFIED_H__

/**
  * @brief Implement of abstract function mon_add_policy for logfile_modified plugin
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t logfile_modified(vmhdlr_t *hdlr, policy_t *policy);

#endif
