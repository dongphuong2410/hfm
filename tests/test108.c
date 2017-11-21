#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hfm.h"
#include "config.h"
#include "private.h"
#include "log.h"

config_t *config;

int main(int argc, char **argv)
{
    log_init(LV_DEBUG, LOG_CONSOLE);
    config = config_init("hfm.cfg");
    vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "windows", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(0, LV_ERROR, "Failed to init domain %s", vmhdlr->name);
        free(vmhdlr);
        return -1;
    }
    hfm_monitor_syscall(vmhdlr, "NtCreateFile", NULL, NULL);
    hfm_monitor_syscall(vmhdlr, "NtOpenFile", NULL, NULL); //Only one remapped table created even if we set the traps at the same page
    hfm_close(vmhdlr);
    free(vmhdlr);
}
