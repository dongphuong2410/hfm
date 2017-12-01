#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "policy.h"
#include "log.h"

char * _trim_left(char *str);
severity_t _lookup_severity(const char *str);
monitor_t _lookup_action(const char *str);
uint8_t _parse_options(char *str);
void _free_policy(int *id, policy_t *policy, void *data);
int _check_valid(policy_t *policy);

/**
  * Parsing the policy file
  * Policy format : <id> <severity> <action> <filepath> OPTIONS="[RECURSIVE] [EXTRACT] [DIR]"
  */
GHashTable *get_policies(const char *policy_file)
{
    FILE *fp = fopen(policy_file, "r");
    if (!fp) {
        writelog(0, LV_ERROR, "Cannot read policy file");
        goto done;
    }

    GHashTable *hashtable = g_hash_table_new(g_int_hash, g_int_equal);;
    char buff[STR_BUFF];
    int linecnt = 0;
    while (fgets(buff, STR_BUFF, fp)) {
        linecnt++;
        char *line = _trim_left(buff);
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')         //Ignore comment or empty line
            continue;

        char *token = strtok(line, " \t\r\n");
        policy_t *new_policy = (policy_t *)calloc(1, sizeof(policy_t));
        new_policy->id          = atoi(token);
        new_policy->severity    = _lookup_severity(strtok(NULL, " \t\r\n"));
        new_policy->type        = _lookup_action(strtok(NULL, " \t\r\n"));
        char *filepath = strtok(NULL, " \t\r\n");
        if (filepath)
            strcpy(new_policy->path, filepath);
        new_policy->options     = _parse_options(strtok(NULL, "\r\n"));
        if (_check_valid(new_policy)) {
            g_hash_table_insert(hashtable, &(new_policy->id), new_policy);
        }
        else {
            writelog(0, LV_WARN, "Invalid policy statement at line %d\n", linecnt);
            free(new_policy);
        }
    }
    fclose(fp);
done:
    return hashtable;
}

void free_policies(GHashTable *hashtable)
{
    g_hash_table_foreach(hashtable, (GHFunc)_free_policy, NULL);
    g_hash_table_destroy(hashtable);
}

char * _trim_left(char *str)
{
    char *startp = str;
    while (*startp == ' ' || *startp == '\t') {
        startp++;
    }
    return startp;
}

int strcicmp(const char *a, const char *b)
{
    for (;;a++,b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a || !*b)
            return d;
    }
}

severity_t _lookup_severity(const char *str)
{
    if (!strcicmp(str, "WARNING")) {
        return WARNING;
    }
    else if (!strcicmp(str, "CRITICAL")) {
        return CRITICAL;
    }
    return SEVERITY_INVALID;
}

monitor_t _lookup_action(const char *str)
{
    if (!strcicmp(str, "CREATE")) {
        return MON_CREATE;
    }
    else if (!strcicmp(str, "DELETE")) {
        return MON_DELETE;
    }
    else if (!strcicmp(str, "MODIFY_CONTENT")) {
        return MON_MODIFY_CONTENT;
    }
    else if (!strcicmp(str, "CHANGE_ATTR")) {
        return MON_CHANGE_ATTR;
    }
    else if (!strcicmp(str, "CHANGE_ACCESS")) {
        return MON_CHANGE_ACCESS;
    }
    return MON_INVALID;
}

uint8_t _parse_options(char *str)
{
    uint8_t options = 0;
    char *opt = strtok(str, " \t\n\"");
    while (opt) {
        if (!strcicmp(opt, "EXTRACT")) {
            options |= POLICY_OPTIONS_EXTRACT;
        }
        opt = strtok(NULL, " \t\"\n");
    }
    return options;
}

void _free_policy(int *id, policy_t *policy, void *data)
{
    free(policy);
}

int _check_valid(policy_t *policy)
{
    if (policy->id == 0
            || policy->severity == SEVERITY_INVALID
            || policy->type == MON_INVALID
            || policy->path[0] == '\0'
            )
            return 0;
    else
        return 1;
}
