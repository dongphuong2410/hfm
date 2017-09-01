#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "policy.h"
#include "log.h"
//#include "private.h"

#define MAX_LEN 1024

char * _trim_left(char *str);
severity_t _lookup_severity(const char *str);
monitor_t _lookup_action(const char *str);
uint8_t _parse_options(char *str);
void _free_policy(policy_t *policy);
int _check_valid(policy_t *policy);

/**
  * Parsing the policy file
  * Policy format : <id> <severity> <action> <filepath> OPTIONS="[RECURSIVE] [EXTRACT] [DIR]"
  */
GSList *get_policies(const char *policy_file)
{
    FILE *fp = fopen(policy_file, "r");
    if (!fp) {
        writelog(LV_ERROR, "Cannot read policy file");
        goto done;
    }

    GSList *list = NULL;
    char buff[MAX_LEN];
    int linecnt = 0;
    while (fgets(buff, MAX_LEN, fp)) {
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
            list = g_slist_append(list, new_policy);
        }
        else {
            writelog(LV_WARN, "Invalid policy statement at line %d\n", linecnt);
            free(new_policy);
        }
    }
    fclose(fp);
done:
    return list;
}

void free_policies(GSList *list)
{
    g_slist_foreach(list, (GFunc)_free_policy, NULL);
    g_slist_free(list);
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
    else if (!strcicmp(str, "MODIFY_LOGFILE")) {
        return MON_MODIFY_LOGFILE;
    }
    else if (!strcicmp(str, "CHANGE_ATTR_READONLY")) {
        return MON_CHANGE_ATTR_READONLY;
    }
    else if (!strcicmp(str, "CHANGE_ATTR_PERMISSION")) {
        return MON_CHANGE_ATTR_PERMISSION;
    }
    else if (!strcicmp(str, "CHANGE_ATTR_OWNERSHIP")) {
        return MON_CHANGE_ATTR_OWNERSHIP;
    }
    else if (!strcicmp(str, "CHANGE_ATTR_HIDDEN")) {
        return MON_CHANGE_ATTR_HIDDEN;
    }
    return MON_INVALID;
}

uint8_t _parse_options(char *str)
{
    uint8_t options = 0;
    char *opt = strtok(str, " \t\n\"");
    while (opt) {
        if (!strcicmp(opt, "DIR")) {
            options |= POLICY_OPTIONS_DIR;
        }
        else if (!strcicmp(opt, "RECURSIVE")) {
            options |= POLICY_OPTIONS_RECURSIVE;
        }
        else if (!strcicmp(opt, "EXTRACT")) {
            options |= POLICY_OPTIONS_EXTRACT;
        }
        opt = strtok(NULL, " \t\"\n");
    }
    return options;
}

void _free_policy(policy_t *policy)
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
