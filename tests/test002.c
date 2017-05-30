#include <stdio.h>
#include <stdlib.h>

#include "policy.h"
#include "log.h"

int main(int argc, char *argv)
{
    log_init(LV_DEBUG, LOG_CONSOLE);
    const char *policy_file = "test002.pol";
    GSList *list = get_policies(policy_file);
    printf("Number of element %d\n", g_slist_length(list));

    free_policies(list);
}
