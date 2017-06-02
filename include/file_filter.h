/**
  * @file file_filter.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Implement functions related to file filtering
  */
#ifndef __HFM_FILE_FILTER_H__
#define __HFM_FILE_FILTER_H__

#include <stdint.h>

typedef struct _filter filter_t;

/**
  * @brief  Init the file filter and return the handler
  * @return filter_t handler
  */
filter_t *filter_init(void);

/**
  * @brief Close the filter
  * @param filter handler
  */
void filter_close(filter_t *filter);

/**
  * @brief Add a pattern
  * @param filter Handler
  * @param pattern Pattern string
  * @return 0 if success, others if error
  */
int filter_add(filter_t *filter, const char *pattern, int id);

/**
  * @brief Check if a filepath/directory path matching with the pattern
  * @param filter Filter handler
  * @param filepath Filepath string
  * @return number of matching pattern
  */
int filter_match(filter_t *filter, const char *filepath, int *arr);

#endif
