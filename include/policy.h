/**
  * @file policy.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Parsing policies from policy rules file
  */
#ifndef __HFM_POLICY_H__
#define __HFM_POLICY_H__

#include <glib.h>

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
