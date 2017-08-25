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
    log_init(LV_ERROR, LOG_CONSOLE);
    config = config_init("vista32.cfg");
    vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
    strncpy(vmhdlr->name, "vista32", STR_BUFF);
    if (FAIL == hfm_init(vmhdlr)) {
        writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
    }
    else {
        printf("OBJECT_ATTRIBUTES__OBJECT_NAME %lu\n", vmhdlr->offsets[OBJECT_ATTRIBUTES__OBJECT_NAME]);
        printf("OBJECT_ATTRIBUTES__ROOT_DIRECTORY %lu\n", vmhdlr->offsets[OBJECT_ATTRIBUTES__ROOT_DIRECTORY]);
        printf("UNICODE_STRING__LENGTH %lu\n", vmhdlr->offsets[UNICODE_STRING__LENGTH]);
        printf("UNICODE_STRING__BUFFER %lu\n", vmhdlr->offsets[UNICODE_STRING__BUFFER]);
        printf("IO_STATUS_BLOCK__INFORMATION %lu\n", vmhdlr->offsets[IO_STATUS_BLOCK__INFORMATION]);
        printf("IO_STATUS_BLOCK__STATUS %lu\n", vmhdlr->offsets[IO_STATUS_BLOCK__STATUS]);
        printf("KPCR__PRCB %lu\n", vmhdlr->offsets[KPCR__PRCB]);
        printf("KPCR__PRCB_DATA %lu\n", vmhdlr->offsets[KPCR__PRCB_DATA]);
        printf("KPRCB__CURRENT_THREAD %lu\n", vmhdlr->offsets[KPRCB__CURRENT_THREAD]);
        printf("KTHREAD__PROCESS %lu\n", vmhdlr->offsets[KTHREAD__PROCESS]);
        printf("HANDLE_TABLE__HANDLE_COUNT %lu\n", vmhdlr->offsets[HANDLE_TABLE__HANDLE_COUNT]);
        printf("HANDLE_TABLE__TABLE_CODE %lu\n", vmhdlr->offsets[HANDLE_TABLE__TABLE_CODE]);
        printf("HANDLE_TABLE_ENTRY__OBJECT %lu\n", vmhdlr->offsets[HANDLE_TABLE_ENTRY__OBJECT]);
        printf("OBJECT_HEADER__BODY %lu\n", vmhdlr->offsets[OBJECT_HEADER__BODY]);
        printf("OBJECT_HEADER__TYPE_INDEX %lu\n", vmhdlr->offsets[OBJECT_HEADER__TYPE_INDEX]);
        printf("OBJECT_HEADER_NAME_INFO__NAME %lu\n", vmhdlr->offsets[OBJECT_HEADER_NAME_INFO__NAME]);
        printf("FILE_OBJECT__FILE_NAME %lu\n", vmhdlr->offsets[FILE_OBJECT__FILE_NAME]);
        printf("FILE_OBJECT__DEVICE_OBJECT %lu\n", vmhdlr->offsets[FILE_OBJECT__DEVICE_OBJECT]);
        printf("FILE_OBJECT__VPB %lu\n", vmhdlr->offsets[FILE_OBJECT__VPB]);
        printf("FILE_OBJECT__SECTION_OBJECT_POINTER %lu\n", vmhdlr->offsets[FILE_OBJECT__SECTION_OBJECT_POINTER]);
        printf("VPB__VOLUME_LABEL %lu\n", vmhdlr->offsets[VPB__VOLUME_LABEL]);
        printf("VPB__VOLUME_LABEL_LENGTH %lu\n", vmhdlr->offsets[VPB__VOLUME_LABEL_LENGTH]);
        printf("EPROCESS__PEB %lu\n", vmhdlr->offsets[EPROCESS__PEB]);
        printf("EPROCESS__UNIQUE_PROCESS_ID %lu\n", vmhdlr->offsets[EPROCESS__UNIQUE_PROCESS_ID]);
        printf("EPROCESS__ACTIVE_PROCESS_LINKS %lu\n", vmhdlr->offsets[EPROCESS__ACTIVE_PROCESS_LINKS]);
        printf("EPROCESS__OBJECT_TABLE %lu\n", vmhdlr->offsets[EPROCESS__OBJECT_TABLE]);
        printf("DEVICE_OBJECT__DRIVER_OBJECT %lu\n", vmhdlr->offsets[DEVICE_OBJECT__DRIVER_OBJECT]);
        printf("DRIVER_OBJECT__DRIVER_NAME %lu\n", vmhdlr->offsets[DRIVER_OBJECT__DRIVER_NAME]);
        printf("DEVICE_OBJECT__VPB %lu\n", vmhdlr->offsets[DEVICE_OBJECT__VPB]);
        printf("PEB__PROCESS_PARAMETERS %lu\n", vmhdlr->offsets[PEB__PROCESS_PARAMETERS]);
        printf("RTL_USER_PROCESS_PARAMETERS__CURRENT_DIRECTORY %lu\n", vmhdlr->offsets[RTL_USER_PROCESS_PARAMETERS__CURRENT_DIRECTORY]);
        printf("RTL_USER_PROCESS_PARAMETERS__IMAGE_PATH_NAME %lu\n", vmhdlr->offsets[RTL_USER_PROCESS_PARAMETERS__IMAGE_PATH_NAME]);
        printf("CURDIR__DOS_PATH %lu\n", vmhdlr->offsets[CURDIR__DOS_PATH]);
        printf("SECTION_OBJECT_POINTERS__DATA_SECTION_OBJECT %lu\n", vmhdlr->offsets[SECTION_OBJECT_POINTERS__DATA_SECTION_OBJECT]);
        printf("SECTION_OBJECT_POINTERS__IMAGE_SECTION_OBJECT %lu\n", vmhdlr->offsets[SECTION_OBJECT_POINTERS__IMAGE_SECTION_OBJECT]);
        printf("SECTION_OBJECT_POINTERS__SHARED_CACHE_MAP %lu\n", vmhdlr->offsets[SECTION_OBJECT_POINTERS__SHARED_CACHE_MAP]);
        printf("OBJECT_DIRECTORY__HASH_BUCKETS %lu\n", vmhdlr->offsets[OBJECT_DIRECTORY__HASH_BUCKETS]);
        printf("OBJECT_DIRECTORY__LOCK %lu\n", vmhdlr->offsets[OBJECT_DIRECTORY__LOCK]);
        printf("OBJECT_DIRECTORY_ENTRY__OBJECT %lu\n", vmhdlr->offsets[OBJECT_DIRECTORY_ENTRY__OBJECT]);
        printf("OBJECT_DIRECTORY_ENTRY__CHAIN_LINK %lu\n", vmhdlr->offsets[OBJECT_DIRECTORY_ENTRY__CHAIN_LINK]);
        printf("OBJECT_SYMBOLIC_LINK__DOS_DEVICE_DRIVE_INDEX %lu\n", vmhdlr->offsets[OBJECT_SYMBOLIC_LINK__DOS_DEVICE_DRIVE_INDEX]);
        printf("OBJECT_SYMBOLIC_LINK__LINK_TARGET %lu\n", vmhdlr->offsets[OBJECT_SYMBOLIC_LINK__LINK_TARGET]);
        printf("CONTROL_AREA__SEGMENT %lu\n", vmhdlr->offsets[CONTROL_AREA__SEGMENT]);
        printf("SEGMENT__CONTROL_AREA %lu\n", vmhdlr->offsets[SEGMENT__CONTROL_AREA]);
        printf("SEGMENT__SIZE_OF_SEGMENT %lu\n", vmhdlr->offsets[SEGMENT__SIZE_OF_SEGMENT]);
        printf("SEGMENT__TOTAL_NUMBER_OF_PTES %lu\n", vmhdlr->offsets[SEGMENT__TOTAL_NUMBER_OF_PTES]);
        printf("SUBSECTION__CONTROL_AREA %lu\n", vmhdlr->offsets[SUBSECTION__CONTROL_AREA]);
        printf("SUBSECTION__PTES_IN_SUBSECTION %lu\n", vmhdlr->offsets[SUBSECTION__PTES_IN_SUBSECTION]);
        printf("SUBSECTION__STARTING_SECTOR %lu\n", vmhdlr->offsets[SUBSECTION__STARTING_SECTOR]);
        printf("SUBSECTION__SUBSECTION_BASE %lu\n", vmhdlr->offsets[SUBSECTION__SUBSECTION_BASE]);
        printf("SUBSECTION__NEXT_SUBSECTION %lu\n", vmhdlr->offsets[SUBSECTION__NEXT_SUBSECTION]);

        printf("HANDLE_TABLE_ENTRY SIZE %lu\n", vmhdlr->sizes[HANDLE_TABLE_ENTRY]);
        printf("CONTROL_AREA SIZE %lu\n", vmhdlr->sizes[CONTROL_AREA]);
        printf("FILE_OBJECT SIZE %lu\n", vmhdlr->sizes[FILE_OBJECT]);
        printf("OBJECT_HEADER_NAME_INFO SIZE %lu\n", vmhdlr->sizes[OBJECT_HEADER_NAME_INFO]);
        printf("OBJECT_HEADER_CREATOR_INFO SIZE %lu\n", vmhdlr->sizes[OBJECT_HEADER_CREATOR_INFO]);

        hfm_close(vmhdlr);
        config_close(config);
    }
    free(vmhdlr);
}
