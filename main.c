#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "hfm.h"
#include "log.h"
#include "config.h"
#include "private.h"
#include "policy.h"
#include "libmon.h"

/**
  * Read configuration from command line
  * and merge with config from config file
  */
static config_t *_read_config(int argc, char **argv);
static void usage(void);

/**
  * Parse vmlist string and init the VM handlers (hfm)
  */
static int _init_vms(const char *str_vmlist, vmhdlr_t **);

static void _set_policies(vmhdlr_t *handler, GSList *policies);
static void _monitor_vm(vmhdlr_t *vm);

config_t *config;           /* config handler */
vmhdlr_t *vms[VM_MAX];                /* List of vm handler */
int vmnum;

static struct sigaction act;
static void close_handler(int sig) {
    int i = 0;
    for (i = 0; i < vmnum; i++) {
        vms[i]->interrupted = sig;
    }
}

int main(int argc, char **argv)
{
    GThread *threads[VM_MAX];
    GSList *policies;           /* List of policies */

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

    //Init monitoring modules
    if (mon_init()) {
        goto done;
    }

    //Init logging module
    if (config_get_str(config, "log_file"))
        log_init(LOG_LEVEL, LOG_TEXTFILE, config_get_str(config, "log_file"));
    else
        log_init(LOG_LEVEL, LOG_CONSOLE);

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

    int i;
    //Set policies
    for (i = 0; i < vmnum; i++) {
        _set_policies(vms[i], policies);
    }

    //Start monitoring threads for each vm
    for (i = 0; i < vmnum; i++) {
        threads[i] = g_thread_new(vms[i]->name, (GThreadFunc)_monitor_vm, vms[i]);
    }

    for (i = 0; i < vmnum; i++) {
        g_thread_join(threads[i]);
    }
done:
    free_policies(policies);
    log_close();
    config_close(config);
    mon_close();
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
            default:
                usage();
                exit(1);
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
        vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
        strncpy(vmhdlr->name, token, STR_BUFF);
        if (FAIL == hfm_init(vmhdlr)) {
            writelog(LV_ERROR, "Failed to init domain %s", vmhdlr->name);
            free(vmhdlr);
        }
        else {
            vms[cnt++] = vmhdlr;
        }
        token = strtok(NULL, ",");
    }
    return cnt;
}

static void _set_policies(vmhdlr_t *handler, GSList *policies)
{
    policy_t *test = (policy_t *)calloc(1, sizeof(policy_t));
    strcpy(test->path, "C:/meo/*");

    test->type = MON_DELETE;
    test->id = 10;
    mon_add_policy(handler, test);

    //test->type = MON_CREATE;
    //test->id = 20;
    //mon_add_policy(handler, test);

    test->type = MON_MODIFY_CONTENT;
    test->id = 30;
    mon_add_policy(handler, test);
    free(test);
}

static void _monitor_vm(vmhdlr_t *vm)
{
    while (!vm->interrupted) {
        hfm_listen(vm);
    }
    /* release resources */
    hfm_close(vm);
    free(vm);
}

static void usage(void)
{
    printf("Usage: hfm <options>\n");
    printf( "\t -p <policy file>            Policy file path\n"
            "\t -c <config file>            Config file path\n"
            "\t -l <log file>               Log file path\n"
            "\t -v <vmlist>                 List of VMs to monitor\n");
}
