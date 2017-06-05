#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "log.h"
#include "config.h"
#include "private.h"
#include "policy.h"
#include "hfm.h"

/**
  * Read configuration from command line
  * and merge with config from config file
  */
static config_t *_read_config(int argc, char **argv);

/**
  * Parse vmlist string and init the VM handlers (hfm)
  */
static int _init_vms(const char *str_vmlist, vmhdlr_t **);

config_t *config;           /* config handler */

static struct sigaction act;
int interrupted = 0;
static void close_handler(int sig) {
    interrupted = sig;
}

int main(int argc, char **argv)
{
    vmhdlr_t *vms[VM_MAX];                /* List of vm handler */
    GThread *threads[VM_MAX];
    GSList *policies;           /* List of policies */
    int vmnum;

    //Signal handler
    act.sa_handler = close_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGALRM, &act, NULL);

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
        vmnum = _init_vms(config_get_str(config, "vmlist"), vms);
    }

    //Start monitoring threads for each vm
    int i;
    for (i = 0; i < vmnum; i++) {
        threads[i] = g_thread_new(vms[i]->name, (GThreadFunc)hfm_run, vms[i]);
    }

    for (i = 0; i < vmnum; i++) {
        g_thread_join(threads[i]);
    }
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

int _init_vms(const char *str_vmlist, vmhdlr_t **vms)
{
    int cnt = 0;
    char s[STR_BUFF];
    strncpy(s, str_vmlist, STR_BUFF);
    char *token = strtok(s, ",");
    while (token) {
        if (cnt >= VM_MAX) {
            writelog(LV_WARN, "Number of VMs exceed the quota (%d). Ignore the others", VM_MAX);
            break;
        }
        vms[cnt++] = hfm_init(token);
        token = strtok(NULL, ",");
    }
    return cnt;
}

