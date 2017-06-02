#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "file_filter.h"
#include "fnmatch.h"


#define MAX_PATTERNS 128

struct _filter {
    GSList *patterns;
    int ids[MAX_PATTERNS];
};

void _free_pattern(nodelist_t *nodes);

filter_t *filter_init(void)
{
    filter_t *filter = (filter_t *)calloc(1, sizeof(filter_t));
    return filter;
}

void filter_close(filter_t *filter)
{
    g_slist_foreach(filter->patterns, (GFunc)_free_pattern, NULL);
}

int filter_add(filter_t *filter, const char *pattern, int id)
{
    nodelist_t *nodes = fn_translate(pattern, 1);
    if (nodes) {
        filter->ids[g_slist_length(filter->patterns)] = id;
        filter->patterns = g_slist_append(filter->patterns, nodes);
        return 0;
    }
    return -1;
}

int filter_match(filter_t *filter, const char *filepath, int *arr)
{
    int cnt = 0;
    nodelist_t *nodes = fn_translate(filepath, 0);
    int i;
    for (i = 0; i < g_slist_length(filter->patterns); i++) {
        nodelist_t *pattern = g_slist_nth_data(filter->patterns, i);
        if (!fn_match(pattern, nodes))
            arr[cnt++] = i;
    }
    free(nodes);
    return cnt;
}

void _free_pattern(nodelist_t *nodes)
{
    free(nodes);
}
