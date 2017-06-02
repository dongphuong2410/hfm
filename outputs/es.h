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
  * @param output Handler
  */
void out_es_init(output_t *output, const char *url);

/**
  * @brief Write using es output
  *
  * @param info Output info
  */
void out_es_write(output_t *output, output_info_t *info);

/**
  * @brief Release resources
  *
  * @param info Output info
  */
void out_es_close(output_t *output);

#endif
