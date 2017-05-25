/**
  * @file es.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Log content to es file
  */
#ifndef __HFM_ES_H
#define __HFM_ES_H

#include "output_format.h"

/**
  * @brief Init es output module
  *
  * @param url Elasticsearch database url
  */
void out_es_init(const char *url);

/**
  * @brief Write using es output
  *
  * @param info Output info
  */
void out_es_write(output_info_t *info);

#endif
