#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "private.h"
#include "log.h"
#include "trapmngr.h"

/**
  * @brief Contain list of int3 breakpoints set at same position
  */
typedef struct int3_wrapper_t {
    uint64_t pa;                //Physical address of breakpoint
    uint8_t doubletrap;         //Original instruction at this address is INT3 or not
    GSList *traps;
} int3_wrapper_t;

struct _trapmngr_t {
    GHashTable *remapped_tbl;       /* Key : original frame no, Value : remapped_t */
    GHashTable *breakpoint_tbl;     /* Key : pa, Value : int3_wrapper_t */
    GHashTable *breakpoint_gfn_tbl; /* Key : frame no, Value : GList of traps */
    GHashTable *memaccess_tbl;      /* Key : frame no, Value : memtrap_t */
};

trapmngr_t *tm_init()
{
    trapmngr_t *trapmngr = (trapmngr_t *)calloc(1, sizeof(trapmngr_t));
    trapmngr->remapped_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, free);
    trapmngr->breakpoint_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
    trapmngr->breakpoint_gfn_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, (GDestroyNotify)g_slist_free);
    trapmngr->memaccess_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
    return trapmngr;
}

void tm_add_remapped(trapmngr_t *tm, remapped_t *r)
{
    g_hash_table_insert(tm->remapped_tbl, &r->o, r);
}

remapped_t *tm_find_remapped(trapmngr_t *tm, uint64_t original)
{
    remapped_t *r = g_hash_table_lookup(tm->remapped_tbl, &original);
    if (r)
        return r;
    return NULL;
}

void tm_destroy(trapmngr_t *tm)
{
    GHashTableIter i;
    uint64_t *key;

    //Free remapped_tbl
    g_hash_table_destroy(tm->remapped_tbl);

    //Free breakpoint_tbl
    g_hash_table_destroy(tm->breakpoint_gfn_tbl);
    int3_wrapper_t *int3w;
    ghashtable_foreach(tm->breakpoint_tbl, i, key, int3w) {
        g_slist_free_full(int3w->traps, (GDestroyNotify)free);
    }
    g_hash_table_destroy(tm->breakpoint_tbl);

    //Free memaccess_tbl
    g_hash_table_destroy(tm->memaccess_tbl);

    free(tm);
}

GSList *tm_int3traps_at_pa(trapmngr_t *tm, uint64_t pa)
{
    int3_wrapper_t *w = g_hash_table_lookup(tm->breakpoint_tbl, &pa);
    return (w == NULL ? NULL : w->traps);
}

void tm_add_int3trap(trapmngr_t *tm, uint64_t pa, trap_t *trap)
{
    //Update breakpoint_tbl
    int3_wrapper_t *w = g_hash_table_lookup(tm->breakpoint_tbl, &pa);
    if (!w) {
        w = (int3_wrapper_t *)calloc(1, sizeof(int3_wrapper_t));
        w->pa = pa;
        g_hash_table_insert(tm->breakpoint_tbl, g_memdup(&pa, sizeof(uint64_t)), w);
    }
    w->traps = g_slist_append(w->traps, trap);

    //Update breakpoint_gfn_tbl
    uint64_t gfn = pa >> PAGE_OFFSET_BITS;
    GSList *traps_at_gfn = g_hash_table_lookup(tm->breakpoint_gfn_tbl, &gfn);
    if (!traps_at_gfn) {
        traps_at_gfn = g_slist_append(traps_at_gfn, trap);
        g_hash_table_insert(tm->breakpoint_gfn_tbl, g_memdup(&gfn, sizeof(uint64_t)), traps_at_gfn);
    }
    else {
        traps_at_gfn = g_slist_append(traps_at_gfn, trap);
    }
}

uint8_t tm_check_doubletrap(trapmngr_t *tm, uint64_t pa)
{
    //TODO: because this function is often called after the tm_int3traps_at_pa, we should cache a wrapper, so that don't have to
    //lookup the hashtable again
    int3_wrapper_t *w = g_hash_table_lookup(tm->breakpoint_tbl, &pa);
    return w->doubletrap;
}

void tm_set_doubletrap(trapmngr_t *tm, uint64_t pa, uint8_t doubletrap)
{
    int3_wrapper_t *w = g_hash_table_lookup(tm->breakpoint_tbl, &pa);
    w->doubletrap = doubletrap;
}

GSList *tm_int3traps_at_gfn(trapmngr_t *tm, uint64_t gfn)
{
    return g_hash_table_lookup(tm->breakpoint_gfn_tbl, &gfn);
}

memtrap_t *tm_find_memtrap(trapmngr_t *tm, uint64_t gfn)
{
    return g_hash_table_lookup(tm->memaccess_tbl, &gfn);
}

void tm_add_memtrap(trapmngr_t *tm, uint64_t *gfn, memtrap_t *wrapper)
{
    g_hash_table_insert(tm->memaccess_tbl, gfn, wrapper);
}

int tm_trap_exist(trapmngr_t *tm, uint64_t pa)
{
    return (NULL != g_hash_table_lookup(tm->breakpoint_tbl, &pa));
}

GSList *tm_all_remappeds(trapmngr_t *tm) {
    GSList *res = NULL;
    GHashTableIter i;
    uint64_t *key;
    remapped_t *remapped = NULL;
    ghashtable_foreach(tm->remapped_tbl, i, key, remapped) {
        res = g_slist_append(res, remapped);
    }
    return res;
}

GList *tm_all_memtraps(trapmngr_t *tm)
{
    return g_hash_table_get_keys(tm->memaccess_tbl);
}
