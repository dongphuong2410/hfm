#include "console.h"


void out_console_init(void)
{
    //TODO: store the VM name
}

void out_console_write(output_t *output, output_info_t *info)
{
    printf("time_sec : %u, time_usec : %u, pid : %d, sid : %s, vmid : %d, policy_id : %d, action : %d, filepath : %s, extpath : %s, data : %s\n",
            info->time_sec,
            info->time_usec,
            info->pid,
            info->sid,
            info->vmid,
            info->policy_id,
            info->action,
            info->filepath,
            info->extpath,
            info->data);
}

void out_console_close(output_t *output)
{
}
