/**
  * @file file_modified.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief File_modified monitor plugin
  */

#ifndef __HFM_MONITOR_FILE_MODIFIED_H__
#define __HFM_MONITOR_FILE_MODIFIED_H__

/**
  * @brief Implement of abstract function mon_add_policy for file_modified plugin
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t file_modified(vmhdlr_t *hdlr, policy_t *policy);

#endif
