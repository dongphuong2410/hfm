#include <clastic.h>
#include "es.h"
#include "log.h"

#define STR_BUFF 1024

static char *index = "hfm";

void out_es_init(output_t *output, const char *url, const char *type)
{
    output->cls = clastic_init(url);
    snprintf(output->es_type, 1024, "%s", type);
    if (!output->cls) {
        writelog(0, LV_FATAL, "Error connect to elasticsearch server %s", url);
    }
}

void _escape(char *source, char *dest, size_t maxlen)
{
    dest[0] = '\0';
    char *sptr = source;
    char *dptr = dest;
    while (*sptr && sptr - source < maxlen) {
        if (*sptr == '\\') {
            *dptr = '\\';
            dptr++;
            *dptr ='\\';
            dptr++;
        }
        else {
            *dptr = *sptr;
            dptr++;
        }
        sptr++;
    }
    *dptr = *sptr;
}

void out_es_write(output_t *output, output_info_t *info)
{
    char query[2048];
    char path_escaped[STR_BUFF];
    _escape(info->filepath, path_escaped, STR_BUFF);
    snprintf(query, 2048, "{\"time_sec\" : \"%u\",      \
                    \"time_usec\" : \"%u\",             \
                    \"pid\" : \"%d\",                   \
                    \"sid\" : \"%s\",                   \
                    \"vmid\" : \"%d\",                  \
                    \"policy_id\" : \"%d\",             \
                    \"action\" : \"%d\",                \
                    \"filepath\" : \"%s\",              \
                    \"extpath\" : \"%s\",               \
                    \"data\" : \"%s\"}",
                    info->time_sec,
                    info->time_usec,
                    info->pid,
                    info->sid,
                    info->vmid,
                    info->policy_id,
                    info->action,
                    path_escaped,
                    info->extpath,
                    info->data);
    clastic_insert(output->cls, index, output->es_type, NULL, query);
}

void out_es_close(output_t *output)
{
    clastic_destroy(output->cls);
}
