#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hfm.h"
#include "private.h"
#include "log.h"

vmhdlr_t *hfm_init(char *vm)
{
    writelog(LV_DEBUG, "Init vm %s", vm);
    vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, vm, STR_BUFF);
    return vmhdlr;
}

hfm_status_t hfm_set_policies(vmhdlr_t *vm, GSList *policies)
{
    return FAIL;
}

hfm_status_t hfm_run(vmhdlr_t *vm)
{
    while (1) {
        printf("VM %s is running\n", vm->name);
        sleep(5);
    }
    return FAIL;
}

void hfm_close(vmhdlr_t *vm)
{
    free(vm);
}
