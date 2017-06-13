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
  * @param vm Pointer to vmhdlr_t struct
  * @return Return pointer to trapmngr_t if init success, or else return NULL
  */
trapmngr_t *traps_init(vmhdlr_t *vm);

/**
  * @brief Destroy trap manager, releases all resources
  * @param Pointer to trapmngr_t
  */
void traps_destroy(trapmngr_t *traps);

/**
  * @brief Add a remapped page to remapped hashtable
  * @param traps Pointer to trapmng_t
  * @param original Original gfn
  * @param remapped Remapped gfn
  */
void traps_add_remapped(trapmngr_t *traps, uint64_t original, uint64_t remapped);

#endif
