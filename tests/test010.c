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
    config = config_init("win7_64.cfg");
    vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "windows", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
        free(vmhdlr);
        return -1;
    }
    hfm_close(vmhdlr);
    free(vmhdlr);
}
