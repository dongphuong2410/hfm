/**
  * @file console.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Log content to console
  */
#ifndef __HFM_CONSOLE_H
#define __HFM_CONSOLE_H

#include "output_format.h"

/**
  * @brief Init console output module
  */
void out_console_init(void);

/**
  * @brief Write using console output
  *
  * @param info Output info
  */
void out_console_write(output_info_t *info);

#endif
