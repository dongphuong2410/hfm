/**
  * @file traps.h
  * @author phuong.do
  * @date 2017-06-13
  * @brief Managing all the information about traps in the system
  */
#ifndef __TRAPS_H__
#define __TRAPS_H__

#include "private.h"

/**
  * @brief Init trap manager
  * @return Return pointer to trapmngr_t if init success, or else return NULL
  */
trapmngr_t *traps_init(void);

/**
  * @brief Destroy trap manager, releases all resources
  * @param Pointer to trapmngr_t
  */
void traps_destroy(trapmngr_t *traps);

#endif
