/**
  * @file log.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Implement functions related to file filtering
  */
#ifndef __HFM_FILE_FILTER_H__
#define __HFM_FILE_FILTER_H__

#include <stdint.h>

/**
  * @brief Check if a filepath/directory path matching with the pattern
  * @param filename Filename string
  * @param pattern Pattern string
  * @return 1 if match, 0 if not match
  */
int8_t match_file(const char *filename, const char *pattern);

#endif
