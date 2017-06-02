#include "console.h"


void out_console_init(void)
{
}

void out_console_write(output_t *output, output_info_t *info)
{
    printf("time : %s, pid : %d, vmid : %d, policy_id : %d, action : %s, filepath : %s, extpath : %s\n",
            info->time,
            info->pid,
            info->vmid,
            info->policy_id,
            action_tostr(info->action),
            info->filepath,
            info->extpath);
}

void out_console_close(output_t *output)
{
}
