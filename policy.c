#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "policy.h"
#include "log.h"
#include "private.h"

#define MAX_LEN 1024

char * _trim_left(char *str);
severity_t _convert_severity(const char *str);
monitor_t _convert_action(const char *str);
uint8_t _convert_options(char *str);
void _free_policy(policy_t *policy);

GSList *get_policies(const char *policy_file)
{
    FILE *fp = fopen(policy_file, "r");
    if (!fp) {
        writelog(LV_ERROR, "Cannot read policy file");
        goto done;
    }

    GSList *list = NULL;
    char buff[MAX_LEN];
    while (fgets(buff, MAX_LEN, fp)) {
        char *line = _trim_left(buff);
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')         //Ignore comment or empty line
            continue;

        char *token = strtok(line, " \t");
        int id = atoi(token);
        severity_t severity = _convert_severity(strtok(NULL, " \t"));
        monitor_t action = _convert_action(strtok(NULL, " \t"));
        char *filepath = strdup(strtok(NULL, " \t"));
        uint8_t options = _convert_options(strtok(NULL, "\r\n"));
        if (id == 0
                || severity == SEVERITY_INVALID
                || action == MON_INVALID
                || filepath == NULL
                || options == -1) {
            writelog(LV_WARN, "Invalid policy statement : %s", line);
            free(filepath);
            continue;
        }
        policy_t *one_policy = (policy_t *)malloc(sizeof(policy_t));
        one_policy->id = id;
        one_policy->type = action;
        one_policy->options = options;
        strcpy(one_policy->path, filepath);
        one_policy->severity = severity;
        list = g_slist_append(list, one_policy);
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

severity_t _convert_severity(const char *str)
{
    if (!strcicmp(str, "WARNING")) {
        return WARNING;
    }
    else if (!strcicmp(str, "CRITICAL")) {
        return CRITICAL;
    }
    return SEVERITY_INVALID;
}

monitor_t _convert_action(const char *str)
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

uint8_t _convert_options(char *str)
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
