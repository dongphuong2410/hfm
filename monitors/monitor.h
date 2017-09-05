/**
  * @file monitor.h
  * @author phuong.do
  * @date 2017-09-05
  * @brief File_created monitor plugin
  */
#ifndef __HFM_MONITORS_MONITOR_H__
#define __HFM_MONITORS_MONITOR_H__

#include "policy.h"
#include "private.h"

/**
  * @brief Implement of abstract function mon_add_policy for file_crated plugin
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t file_created_add_policy(vmhdlr_t *hdlr, policy_t *policy);

/**
  * @brief Close
  */
void file_created_close(void);

/**
  * @brief Implement of abstract function mon_add_policy for file_deleted plugin_
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t file_deleted_add_policy(vmhdlr_t *hdlr, policy_t *policy);

/**
  * @brief Close
  */
void file_deleted_close(void);

/**
  * @brief Implement of abstract function mon_add_policy for file_modified plugin
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t file_modified_add_policy(vmhdlr_t *hdlr, policy_t *policy);

/**
  * @brief Close
  */
void file_modified_close(void);

/**
  * @brief Implement of abstract function mon_add_policy for attr_changed plugin_
  *
  * @param hdlr vmhdlr pointer
  * @param policy Pointer to policy
  * @return FAIL or success
  */
hfm_status_t attr_changed_add_policy(vmhdlr_t *hdlr, policy_t *policy);

/**
  * @brief Close
  */
void attr_changed_close(void);


#endif

