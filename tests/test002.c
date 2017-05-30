#include <stdio.h>
#include <stdlib.h>

#include "policy.h"
#include "private.h"
#include "log.h"

void _print_policy(policy_t *policy);

int main(int argc, char *argv)
{
    const char *policy_file = "test002.pol";
    GSList *list = get_policies(policy_file);
    printf("%d\n", g_slist_length(list));

    g_slist_foreach(list, (GFunc)_print_policy, NULL);

    free_policies(list);
}

void _print_policy(policy_t *policy)
{
    printf("%d\n", policy->id);
    printf("%d\n", policy->severity);
    printf("%d\n", policy->type);
    printf("%d\n", policy->options);
    printf("%s\n", policy->path);
}
