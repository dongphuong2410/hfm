#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xenctrl.h>

#include "hfm.h"
#include "private.h"
#include "log.h"
#include "vmi_helper.h"
#include "xen_helper.h"
#include "strace.h"     //TODO : Temporary here

extern int interrupted;

vmhdlr_t *hfm_init(char *vm)
{
    vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, vm, STR_BUFF);
    if ((vmhdlr->xen = xen_init_interface(vmhdlr->name)) == NULL) {
        writelog(LV_ERROR, "Failed to init XEN on domain %s", vmhdlr->name);
        goto error_init_xen;
    }
    if (FAIL == vh_init(vmhdlr)) {
        writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
        goto error_init_vh;
    }
    strace_register(vmhdlr, "NtCreateFile");
    goto done;

error_init_vh:
    xen_free_interface(vmhdlr->xen);
error_init_xen:
    free(vmhdlr);
    vmhdlr = NULL;
done:
    return vmhdlr;
}

hfm_status_t hfm_set_policies(vmhdlr_t *vm, GSList *policies)
{
    return FAIL;
}

hfm_status_t hfm_run(vmhdlr_t *vm)
{
    while (!interrupted) {
        printf("VM %s is running\n", vm->name);
        sleep(5);
    }
    hfm_close(vm);
    printf("VM %s exitted safely\n", vm->name);
    return FAIL;
}

void hfm_close(vmhdlr_t *vm)
{
    vh_close(vm);
    xen_free_interface(vm->xen);
    free(vm);
}
