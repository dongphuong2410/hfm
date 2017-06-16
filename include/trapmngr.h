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
    uint64_t pa;
    GSList *traps;
} int3_wrapper_t;

/**
  * @brief  Information about mem trap 
  */
typedef struct mem_wrapper_t {
} mem_wrapper_t;

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
  * @param remapped_t
  */
void trapmngr_add_remapped(trapmngr_t *trapmngr, remapped_t *remapped);

/**
  * @brief Find a remapped page of a frame
  * @param trapmngr Pointer to trapmngr
  * @param origin Original frame number
  * @return Return address of remap of this frame, or return 0 if not exist
  */
remapped_t *trapmngr_find_remapped(trapmngr_t *trapmngr, uint64_t original);

/**
  * @brief Find a breakpoint wrapper at physical address
  * @param trapmngr Pointer to trapmngr_t
  * @param pa Physical address
  * @return Pointer to breakpoint wrapper
  */
int3_wrapper_t *trapmngr_find_breakpoint(trapmngr_t *trapmngr, uint64_t pa);

/**
  * @brief Add a breakpoint wrapper at physical address to hashtable
  * @param trapmngr Pointer to trapmngr_t
  * @param pa Physical address of breakpoints
  * @param wrapper Pointer to a int3_wrapper_t
  */
void trapmngr_add_breakpoint(trapmngr_t *trapmngr, uint64_t *pa, int3_wrapper_t *wrapper);

/**
  * @brief Find a breakpoint wrapper in a page
  * @param trapmngr Pointer to trapmngr_t
  * @param gfn GFN
  * @return List of traps
  */
GSList *trapmngr_find_breakpoint_gfn(trapmngr_t *trapmngr, uint64_t gfn);

/**
  * @brief Add a GList of traps  in a page to hashtable
  * @param trapmngr Pointer to trapmngr_t
  * @param gfn Frame number
  * @param traps List of traps
  */
void trapmngr_add_breakpoint_gfn(trapmngr_t *trapmngr, uint64_t *gfn, GSList *traps);

/**
  * @brief Find a memaccess wrapper at frame number
  * @param trapmngr Pointer to trapmngr_t
  * @param gfn Frame number
  * @return Pointer to memaccess wrapper
  */
mem_wrapper_t *trapmngr_find_memtrap(trapmngr_t *trapmngr, uint64_t gfn);

/**
  * @brief Add a memaccess wrapper at frame number
  * @param trapmngr Pointer to trapmngr_t
  * @gfn Frame number
  * @param wrapper Pointer to memaccess wrapper to add
  */
void trapmngr_add_memtrap(trapmngr_t *trapmngr, uint64_t *gfn, mem_wrapper_t *wrapper);

#endif
