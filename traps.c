#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <libvmi/slat.h>
#include <glib.h>

#include "private.h"
#include "log.h"
#include "traps.h"
#include "xen_helper.h"

struct _trapmngr_t {
    vmhdlr_t *handler;
    GHashTable *remapped_tbl;
    GHashTable *breakpoint_tbl;
};

typedef struct _remapped_t {
    uint64_t o;     //Original address
    uint64_t r;     //Remapped address
} remapped_t;

trapmngr_t *traps_init(vmhdlr_t *vm)
{
    trapmngr_t *traps = (trapmngr_t *)calloc(1, sizeof(trapmngr_t));
    traps->handler = vm;
    traps->remapped_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, free);
    traps->breakpoint_tbl =g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
    return traps;
}

void traps_add_remapped(trapmngr_t *trap_manager, uint64_t original, uint64_t remapped)
{
    remapped_t *r = (remapped_t *)calloc(1, sizeof(remapped_t));
    r->o = original;
    r->r = remapped;
    g_hash_table_insert(trap_manager->remapped_tbl, &original, r);
}

uint64_t traps_find_remapped(trapmngr_t *trap_manager, uint64_t original)
{
    remapped_t *r = g_hash_table_lookup(trap_manager->remapped_tbl, &original);
    if (r)
        return r->r;
    return 0;
}

void traps_destroy(trapmngr_t *trap_manager)
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
    g_hash_table_destroy(trap_manager->breakpoint_tbl);

    free(trap_manager);
}

int3break_t *traps_find_breakpoint(trapmngr_t *trap_manager, uint64_t pa)
{
    return g_hash_table_lookup(trap_manager->breakpoint_tbl, &pa);
}

void traps_add_breakpoint(trapmngr_t *trap_manager, uint64_t *pa, int3break_t *wrapper)
{
    g_hash_table_insert(trap_manager->breakpoint_tbl, pa, wrapper);
}
