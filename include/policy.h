/**
  * @file policy.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Parsing policies from policy rules file
  */
#ifndef __HFM_POLICY_H__
#define __HFM_POLICY_H__

#include <glib.h>
#include <stdint.h>

#include "constants.h"

#define POLICY_OPTIONS_DIR 0x01             /** Policy options flag : this is directory */
#define POLICY_OPTIONS_RECURSIVE 0x02       /** Policy options flag : allow recursive */
#define POLICY_OPTIONS_EXTRACT 0x04       /** Policy options flag : allow file extracting */

/**
  * @brief Policy severity
  */
typedef enum {
    SEVERITY_INVALID,
    WARNING,
    CRITICAL
} severity_t;

/**
  * @brief monitoring plugin id
  */
typedef enum {
    MON_INVALID,
    MON_CREATE,
    MON_DELETE,
    MON_MODIFY_CONTENT,
    MON_MODIFY_LOGFILE,
    MON_CHANGE_ATTR,
    MON_CHANGE_ATTR_READONLY,
    MON_CHANGE_ATTR_PERMISSION,
    MON_CHANGE_ATTR_OWNERSHIP,
    MON_CHANGE_ATTR_HIDDEN
} monitor_t;

typedef struct _policy {
    int id;
    char path[PATH_MAX_LEN];
    monitor_t type;
    uint8_t options;
    severity_t severity;
} policy_t;

/**
  * @brief Parse policy file and return list of policies
  *
  * @param policy_file Path of policy file
  * @return List of policies (policy_t)
  */
GSList *get_policies(const char *policy_file);

/**
  * @brief Free policy list after done with it
  *
  * @param list List of policies obtained by calling get_policies
  */
void free_policies(GSList *list);


#endif
