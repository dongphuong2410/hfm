/**
  * @file trapmngr.h
  * @author phuong.do
  * @date 2017-06-13
  * @brief Managing all the information about traps in the system
  */
#ifndef __TRAPS_H__
#define __TRAPS_H__

#include "private.h"

/**
  * @brief Contain list of int3 breakpoints set at same position
  */
typedef struct int3_wrapper_t {
    GSList *traps;
} int3_wrapper_t;

/**
  * @brief Contain a mapping address pair
  */
typedef struct _remapped_t {
    uint64_t o;     //Original address
    uint64_t r;     //Remapped address
} remapped_t;

/**
  * @brief Init trap manager
  * @param vm Pointer to vmhdlr_t struct
  * @return Return pointer to trapmngr_t if init success, or else return NULL
  */
trapmngr_t *trapmngr_init(vmhdlr_t *vm);

/**
  * @brief Destroy trap manager, releases all resources
  * @param Pointer to trapmngr_t
  */
void trapmngr_destroy(trapmngr_t *trapmngr);

/**
  * @brief Add a remapped page to remapped hashtable
  * @param trapmngr Pointer to trapmng_t
  * @param original Original gfn
  * @param remapped Remapped gfn
  */
void trapmngr_add_remapped(trapmngr_t *trapmngr, uint64_t original, uint64_t remapped);

/**
  * @brief Find a remapped page of a frame
  * @param trapmngr Pointer to trapmngr
  * @param origin Original frame number
  * @return Return address of remap of this frame, or return 0 if not exist
  */
uint64_t trapmngr_find_remapped(trapmngr_t *trapmngr, uint64_t original);

int3_wrapper_t *trapmngr_find_breakpoint(trapmngr_t *trapmngr, uint64_t pa);

void trapmngr_add_breakpoint(trapmngr_t *trapmngr, uint64_t *pa, int3_wrapper_t *wrapper);
#endif
