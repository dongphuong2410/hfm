#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hfm.h"
#include "config.h"
#include "private.h"
#include "log.h"
#include "win.h"
#include "win_offsets.h"

config_t *config;

int main(int argc, char **argv)
{
    GSList *it = NULL;
    vmhdlr_t *vmhdlr = NULL;
    log_init(LV_ERROR, LOG_CONSOLE);

    config = config_init("win7_64.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "windows", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(0, LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        int i;
        for (i = 0; i < REKALL_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = REKALL_OFFSETS_MAX + 1; i < WIN_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = 0; i < WIN_SIZES_MAX; i++) {
            printf("%s SIZE : %lu\n", win_size_names[i], vmhdlr->sizes[i]);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);

    printf("==========================\n");
    config = config_init("win7_32.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "windows7_32", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(0, LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        int i;
        for (i = 0; i < REKALL_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = REKALL_OFFSETS_MAX + 1; i < WIN_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = 0; i < WIN_SIZES_MAX; i++) {
            printf("%s SIZE : %lu\n", win_size_names[i], vmhdlr->sizes[i]);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);

    printf("==========================\n");
    config = config_init("vista32.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "vista32", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(0, LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        int i;
        for (i = 0; i < REKALL_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = REKALL_OFFSETS_MAX + 1; i < WIN_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = 0; i < WIN_SIZES_MAX; i++) {
            printf("%s SIZE : %lu\n", win_size_names[i], vmhdlr->sizes[i]);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);

    printf("==========================\n");
    config = config_init("server2003.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "server2003", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(0, LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        int i;
        for (i = 0; i < REKALL_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = REKALL_OFFSETS_MAX + 1; i < WIN_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = 0; i < WIN_SIZES_MAX; i++) {
            printf("%s SIZE : %lu\n", win_size_names[i], vmhdlr->sizes[i]);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);

    printf("==========================\n");
    config = config_init("server2008.cfg");
    vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "server2008", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(0, LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        int i;
        for (i = 0; i < REKALL_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = REKALL_OFFSETS_MAX + 1; i < WIN_OFFSETS_MAX; i++) {
            printf("%s %s : %lu\n", win_offset_names[i][0], win_offset_names[i][1], vmhdlr->offsets[i]);
        }
        for (i = 0; i < WIN_SIZES_MAX; i++) {
            printf("%s SIZE : %lu\n", win_size_names[i], vmhdlr->sizes[i]);
        }
        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);
}
