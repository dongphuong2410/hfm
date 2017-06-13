#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "log.h"
#include "traps.h"

struct _trapmngr_t {
    GHashTable *remapped_tbl;
};

trapmngr_t *traps_init()
{
    trapmngr_t *traps = (trapmngr_t *)calloc(1, sizeof(trapmngr_t));
    traps->remapped_tbl = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, free);
}

void traps_destroy(trapmngr_t *traps)
{
    g_hash_table_destroy(traps->remapped_tbl);
    free(traps);
}
