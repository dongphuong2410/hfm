#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hfm.h"
#include "private.h"
#include "log.h"
#include "vmi_helper.h"
#include "strace.h"     //TODO : Temporary here

extern int interrupted;

vmhdlr_t *hfm_init(char *vm)
{
    vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, vm, STR_BUFF);
    if (FAIL == vh_init(vmhdlr)) {
        writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
        goto error;
    }
    goto done;

error:
    free(vmhdlr);
    vmhdlr = NULL;
done:
    return vmhdlr;
}

hfm_status_t hfm_set_policies(vmhdlr_t *vm, GSList *policies)
{
    strace_register(vm, "NtCreateFile");
    return SUCCESS;
}

hfm_status_t hfm_run(vmhdlr_t *vm)
{
    while (!interrupted) {
        vh_listen(vm);
    }
    hfm_close(vm);
    return SUCCESS;
}

void hfm_close(vmhdlr_t *vm)
{
    strace_destroy(vm);
    vh_close(vm);
    free(vm);
}
