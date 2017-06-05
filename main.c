#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "config.h"
#include "private.h"
#include "policy.h"
#include "hfm.h"

/**
  * Read configuration from command line
  * and merge with config from config file
  */
config_t *_read_config(int argc, char **argv);

/**
  * Parse vmlist string and init the VM handlers (hfm)
  */
GSList *_init_vms(const char *str_vmlist);

/**
  * Create a thread for monitoring the vm
  */
void _start_monitor(vmhdlr_t *vm);

config_t *config;           /* config handler */
GSList *policies;           /* List of policies */


int main(int argc, char **argv)
{
    GSList *vms;                /* List of vm handler */

    //Read configuration
    config = _read_config(argc, argv);

    //Init logging module
    if (config_get_str(config, "log_file"))
        log_init(LV_DEBUG, LOG_TEXTFILE, config_get_str(config, "log_file"));
    else
        log_init(LV_DEBUG, LOG_CONSOLE);

    //Get policies
    if (!config_get_str(config, "policy_file")) {
        writelog(LV_WARN, "No policy file specified");
    }
    else {
        policies = get_policies(config_get_str(config, "policy_file"));
        if (g_slist_length(policies) == 0) {
            writelog(LV_WARN, "0 policy read");
        }
    }

    //Init vm lists
    if (!config_get_str(config, "vmlist")) {
        writelog(LV_FATAL, "No vmlist specified");
        goto done;
    }
    else {
        vms = _init_vms(config_get_str(config, "vmlist"));
    }

    //Start monitoring threads for each vm
    g_slist_foreach(vms, (GFunc)_start_monitor, NULL);
done:
    free_policies(policies);
    log_close();
    config_close(config);
    return 0;
}

config_t *_read_config(int argc, char **argv)
{
    char _cfg_file[PATH_MAX_LEN] = "";         //Default config file
    char _policy_file[PATH_MAX_LEN] = "";
    char _log_file[PATH_MAX_LEN] = "";
    char _vmlist[1024] = "";
    int c;

    //Read config from command line
    while ((c = getopt(argc, argv, "p:c:l:v:")) != -1) {
        switch (c) {
            case 'p':
                strncpy(_policy_file, optarg, PATH_MAX_LEN);
                break;
            case 'c':
                strncpy(_cfg_file, optarg, PATH_MAX_LEN);
                break;
            case 'l':
                strncpy(_log_file, optarg, PATH_MAX_LEN);
                break;
            case 'v':
                strncpy(_vmlist, optarg, 1024);
                break;
        }
    }
    //Read config from file
    config_t *cfg = config_init(_cfg_file[0] ? _cfg_file : DEFAULT_CONFIG);

    //Merge config from cmd into cfg (config from cmdline will override configs from file)
    if (_policy_file[0])
        config_set_str(cfg, "policy_file", _policy_file);
    if (_log_file[0])
        config_set_str(cfg, "log_file", _log_file);
    if (_vmlist[0])
        config_set_str(cfg, "vmlist", _vmlist);
    return cfg;
}

GSList *_init_vms(const char *str_vmlist)
{
    GSList *list = NULL;
    char s[STR_BUFF];
    strncpy(s, str_vmlist, STR_BUFF);
    char *token = strtok(s, ",");
    while (token) {
        vmhdlr_t *vmhdlr = hfm_init(token);
        if (vmhdlr)
            list = g_slist_append(list, vmhdlr);
        token = strtok(NULL, ",");
    }
    return list;
}

void _start_monitor(vmhdlr_t *vm)
{
    printf("Start monitor vm %s\n", vm->name);
}
