/**
  * @file file_created.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief File_created monitor plugin
  */

#ifndef __HFM_MONITOR_FILE_CREATED_H__
#define __HFM_MONITOR_FILE_CREATED_H__
#include "private.h"

/**
  * @brief Implement of abstract function mon_add_policy for file_crated plugin
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy);

#endif
