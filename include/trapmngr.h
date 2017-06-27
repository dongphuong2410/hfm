/**
  * @file trapmngr.h
  * @author phuong.do
  * @date 2017-06-13
  * @brief Managing all the information about traps in the system
  */
#ifndef __TRAPS_H__
#define __TRAPS_H__

#include <glib.h>
#include "private.h"

/**
  * @brief Contain list of int3 breakpoints set at same position
  */
typedef struct int3_wrapper_t {
    uint64_t pa;                //Physical address of breakpoint
    uint8_t doubletrap;         //Original instruction at this address is INT3 or not
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
  * @return Return pointer to trapmngr_t if init success, or else return NULL
  */
trapmngr_t *tm_init();

/**
  * @brief Destroy trap manager, releases all resources
  * @param Pointer to trapmngr_t
  */
void tm_destroy(trapmngr_t *tm);

/**
  * @brief Add a remapped page to remapped hashtable
  * @param tm Pointer to trapmng_t
  * @param remapped_t
  */
void tm_add_remapped(trapmngr_t *tm, remapped_t *remapped);

/**
  * @brief Find a remapped page of a frame
  * @param tm Pointer to trapmngr
  * @param origin Original frame number
  * @return Return address of remap of this frame, or return 0 if not exist
  */
remapped_t *tm_find_remapped(trapmngr_t *tm, uint64_t original);

/**
  * @brief Find all breakpoints at a physical address
  * @param tm Pointer to trapmngr_t
  * @param pa Physical address
  * @return GSList of breakpoints at a physical address
  */
GSList *tm_int3traps_at_pa(trapmngr_t *tm, uint64_t pa);

/**
  * @brief Add a breakpoint trap
  * @param tm Pointer to trapmngr_t
  * @param trap Pointer to a breakpoint trap
  */
void tm_add_int3trap(trapmngr_t *tm, trap_t *trap);

/**
  * @brief Remove a breakpoint trap
  * @param tm Pointer to trapmngr_t
  * @param trap Pointer to trap
  * @return Return number of trap remains in the pa
  */
int tm_remove_int3trap(trapmngr_t *tm, trap_t *trap);

/**
  * @brief Find a breakpoint wrapper in a page
  * @param tm Pointer to trapmngr_t
  * @param gfn GFN
  * @return List of traps
  */
GSList *tm_int3traps_at_gfn(trapmngr_t *tm, uint64_t gfn);

/**
  * @brief Find a memaccess wrapper at frame number
  * @param tm Pointer to trapmngr_t
  * @param gfn Frame number
  * @return Pointer to memaccess wrapper
  */
memtrap_t *tm_find_memtrap(trapmngr_t *tm, uint64_t gfn);

/**
  * @brief Add a memaccess wrapper at frame number
  * @param tm Pointer to trapmngr_t
  * @gfn Frame number
  * @param wrapper Pointer to memaccess wrapper to add
  */
void tm_add_memtrap(trapmngr_t *tm, uint64_t *gfn, memtrap_t *wrapper);

/**
  * @brief Check if the instruction at address is doubletrap or not
  * @param tm Pointer to trapmngr_t
  * @param pa Physical address
  * @return 1 if doubletrap, 0 if not
  */
uint8_t tm_check_doubletrap(trapmngr_t *tm, uint64_t pa);

/**
  * @brief Set/unset doubletrap
  * @param tm Pointer to trapmngr_t
  * @param pa Physical address
  * @param doubletrap Is doubletrap or not ?
  */
void tm_set_doubletrap(trapmngr_t *tm, uint64_t pa, uint8_t doubletrap);

/**
  * @brief Check if there is any trap existed at this pa before
  * @param tm Pointer to trapmngr_t
  * @param pa Physical address
  * @return trap exist or not
  */
int tm_trap_exist(trapmngr_t *tm, uint64_t pa);

/**
  * @brief Return all remapped added to trap manager
  * @param tm Pointer to trapmngr_t
  * @return List of remappeds
  */
GSList *tm_all_remappeds(trapmngr_t *tm);

/**
  * @brief Return all memtraps to trap manager
  * @param tm Pointer to trapmngr_t
  * @return List of memtraps
  */
GList *tm_all_memtraps(trapmngr_t *tm);

#endif
