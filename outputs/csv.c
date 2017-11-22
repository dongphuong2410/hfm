#include <stdio.h>
#include <stdlib.h>
#include "console.h"
#include "log.h"
#include "output_format.h"


void out_csv_init(output_t *output, const char *filepath)
{
    output->fp = fopen(filepath, "a");
    if (output->fp == NULL) {
         writelog(0, LV_FATAL, "Error open csv file %s", filepath);
    }
}

void out_csv_write(output_t *output, output_info_t *info)
{
    if (output->fp) {
        fprintf(output->fp, "%u,%u,%d,%d,%d,%d,%s,%s,%s\n",
                info->time_sec,
                info->time_usec,
                info->pid,
                info->vmid,
                info->policy_id,
                info->action,
                info->filepath,
                info->extpath,
                info->data);
    }
}

void out_csv_close(output_t *output)
{
    if (output->fp) fclose(output->fp);
}
