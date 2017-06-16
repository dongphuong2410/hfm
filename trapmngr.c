#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <libvmi/slat.h>
#include <glib.h>

#include "private.h"
#include "log.h"
#include "trapmngr.h"
#include "xen_helper.h"

struct _trapmngr_t {
    vmhdlr_t *handler;
    GHashTable *remapped_tbl;       /* Key : original frame no, Value : remapped_t */
    GHashTable *breakpoint_tbl;     /* Key : pa, Value : int3_wrapper_t */
    GHashTable *breakpoint_gfn_tbl; /* Key : frame no, Value : GList of traps */
    GHashTable *memaccess_tbl;      /* Key : frame no, Value : mem_wrapper_t */
};

trapmngr_t *trapmngr_init(vmhdlr_t *vm)
{
    trapmngr_t *trapmngr = (trapmngr_t *)calloc(1, sizeof(trapmngr_t));
    trapmngr->handler = vm;
    trapmngr->remapped_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, free);
    trapmngr->breakpoint_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
    trapmngr->breakpoint_gfn_tbl = g_hash_table_new(g_int64_hash, g_int64_equal);
    trapmngr->memaccess_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
    return trapmngr;
}

void trapmngr_add_remapped(trapmngr_t *trap_manager, remapped_t *r)
{
    g_hash_table_insert(trap_manager->remapped_tbl, &r->o, r);
}

remapped_t *trapmngr_find_remapped(trapmngr_t *trap_manager, uint64_t original)
{
    remapped_t *r = g_hash_table_lookup(trap_manager->remapped_tbl, &original);
    if (r)
        return r;
    return NULL;
}

void trapmngr_destroy(trapmngr_t *trap_manager)
{
    //Free remapped_tbl
    GHashTableIter i;
    uint64_t *key;
    remapped_t *remapped = NULL;
    ghashtable_foreach(trap_manager->remapped_tbl, i, key, remapped) {
        vmi_slat_change_gfn(trap_manager->handler->vmi, trap_manager->handler->altp2m_idx, remapped->o, ~0);
        xen_free_shadow_frame(trap_manager->handler->xen, &remapped->r);
    }
    g_hash_table_destroy(trap_manager->remapped_tbl);

    //Free breakpoint_tbl
    int3_wrapper_t *int3w;
    ghashtable_foreach(trap_manager->breakpoint_tbl, i, key, int3w) {
        g_slist_free_full(int3w->traps, (GDestroyNotify)free);
    }
    g_hash_table_destroy(trap_manager->breakpoint_tbl);
    GSList *traplist;
    ghashtable_foreach(trap_manager->breakpoint_gfn_tbl, i, key, traplist) {
        g_slist_free(traplist);
    }
    g_hash_table_destroy(trap_manager->breakpoint_gfn_tbl);

    //Free memaccess_tbl
    g_hash_table_destroy(trap_manager->memaccess_tbl);

    free(trap_manager);
}

int3_wrapper_t *trapmngr_find_breakpoint(trapmngr_t *trap_manager, uint64_t pa)
{
    return g_hash_table_lookup(trap_manager->breakpoint_tbl, &pa);
}

void trapmngr_add_breakpoint(trapmngr_t *trap_manager, uint64_t *pa, int3_wrapper_t *wrapper)
{
    g_hash_table_insert(trap_manager->breakpoint_tbl, pa, wrapper);
}

GSList *trapmngr_find_breakpoint_gfn(trapmngr_t *trapmngr, uint64_t gfn)
{
    return g_hash_table_lookup(trapmngr->breakpoint_gfn_tbl, &gfn);
}

void trapmngr_add_breakpoint_gfn(trapmngr_t *trapmngr, uint64_t *gfn, GSList *traps)
{
    g_hash_table_insert(trapmngr->breakpoint_gfn_tbl, gfn, traps);
}

mem_wrapper_t *trapmngr_find_memtrap(trapmngr_t *trapmngr, uint64_t gfn)
{
    return g_hash_table_lookup(trapmngr->memaccess_tbl, &gfn);
}

void trapmngr_add_memtrap(trapmngr_t *trapmngr, uint64_t *gfn, mem_wrapper_t *wrapper)
{
    g_hash_table_insert(trapmngr->memaccess_tbl, gfn, wrapper);
}

