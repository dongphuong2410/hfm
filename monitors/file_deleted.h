/**
  * @file file_deleted.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief File_deleted monitor plugin
  */

#ifndef __HFM_MONITOR_FILE_DELETED_H__
#define __HFM_MONITOR_FILE_DELETED_H__

#include "private.h"
#include "policy.h"

/**
  * @brief Implement of abstract function mon_add_policy for file_deleted plugin_
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t file_deleted_add_policy(vmhdlr_t *hdlr, policy_t *policy);

#endif
