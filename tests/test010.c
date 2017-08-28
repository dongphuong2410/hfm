#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hfm.h"
#include "config.h"
#include "private.h"
#include "log.h"
#include "win.h"

config_t *config;

int main(int argc, char **argv)
{
    GSList *it = NULL;
    vmhdlr_t *vmhdlr;
    log_init(LV_DEBUG, LOG_CONSOLE);

    config = config_init("win7_64.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "windows", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        for (it = vmhdlr->drives; it; it = it->next) {
            drive_t *drive = (drive_t *)it->data;
            printf("%s\n", drive->win_name);
            printf("%s\n", drive->dos_name);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);

    config = config_init("win7_32.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "windows7_32", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        for (it = vmhdlr->drives; it; it = it->next) {
            drive_t *drive = (drive_t *)it->data;
            printf("%s\n", drive->win_name);
            printf("%s\n", drive->dos_name);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);

    config = config_init("vista32.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "vista32", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        for (it = vmhdlr->drives; it; it = it->next) {
            drive_t *drive = (drive_t *)it->data;
            printf("%s\n", drive->win_name);
            printf("%s\n", drive->dos_name);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);
}
